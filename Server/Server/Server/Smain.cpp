#include "server.h"
using namespace std;
#pragma comment(lib,"pthreadVC2.lib")
#pragma comment(lib,"WS2_32.lib")
vector<struct Client> ClientList;
pthread_t QuitThread;
void* Quit_thread(void *sSocket) {		//因为scanf是阻塞式函数，所以如果要不停检测是否输入退出的信号
							//就必须新建一个子进程来循环读取
	SOCKET s;
	char ch=' ';
	s = (SOCKET)sSocket;
	do {
		ch = getchar();
		Sleep(10);
	} while (ch != 'q'&&ch != 'Q');
	closesocket(s);
	WSACleanup();
	for (int i = 0; i < ClientList.size(); ++i) {
		if(ClientList[i].state ==1 )
			ClientList[i].state = 0;
		pthread_cancel(ClientList[i].ClientThread);
	}
	pthread_exit(NULL);
	return 0;
}

void Response(char *BUFFER,int ID) {
	/*
		关于这个BUFFER指针的安全性问题（因为多线程情况下）
		两个主要的问题：1、调用这个指针的时候是否会已经被释放了或者写入了其他东西
		2、这个指针是否会因为在程序中调用别的函数而被别的线程所使用
		认真思考和查阅了资料，这样写应该是没有啥问题的
		因为这个函数一直在子线程当中，并且buffer是属于局部变量，应该是属于独享的栈当中的
		每个子线程调用response这个函数会把一些东西入栈，而buffer应该也是分配在栈上面的，而栈是线程独享的，所以不会出现安全性问题,
		脑袋大了一圈，然后发现人丑就要多读书
	*/
	Client cl = ClientList[ID];
	Client cld;
	char SendBuffer[LEN];
	char tmpBuffer[LEN];
	SYSTEMTIME time_t;
	char id[5];
	char port[128];
	char sourceid[5];
	char desid[5];
	int i;
	int destination;
	int error_code;
	char Error_code[16];
	bool sendflag = true;
	_itoa(ID, sourceid, 10);		//记录是哪个client发送的请求
	memset(SendBuffer, 0, LEN);
	memset(desid, 0, 5);
	memset(tmpBuffer,0,LEN);
	switch (gettype(BUFFER[0])) {
	case 1:
		printf("Client %d is asking for current time\n", ID);
		strcat(SendBuffer, "T");
		strcat(SendBuffer, "Your ID:");
		strcat(SendBuffer, sourceid);
		strcat(SendBuffer, "\n");
		GetLocalTime(&time_t);
		sprintf(tmpBuffer, "%04d-%02d-%02d %02d:%02d:%02d$@",time_t.wYear,time_t.wMonth,time_t.wDay,time_t.wHour,time_t.wMinute,time_t.wSecond);
		strcat(SendBuffer, tmpBuffer);
		if (send(cl.socket, SendBuffer, strlen(SendBuffer), 0) == SOCKET_ERROR) {
			printf("send time error with error code: %d\n", WSAGetLastError());
		}
		else
			printf("send time successfully\n");
		break;
	case 2:
		printf("Client %d is asking for Host name\n", ID);
		strcat(SendBuffer, "N");
		strcat(SendBuffer, "Your ID:");
		strcat(SendBuffer, sourceid);
		strcat(SendBuffer, "\n");
		gethostname(tmpBuffer, 128);
		strcat(SendBuffer, tmpBuffer);
		strcat(SendBuffer, "$@");
		if (send(cl.socket, SendBuffer, strlen(SendBuffer), 0) == SOCKET_ERROR) {
			printf("send hostname error with error code: %d\n", WSAGetLastError());
		}
		else
			printf("send hostname successfully\n");
		break;
	case 3:
		printf("Client %d is asking for Client list\n", ID);
		strcat(SendBuffer, "L");
		strcat(SendBuffer, "Your ID:");
		strcat(SendBuffer, sourceid);
		strcat(SendBuffer, "\n");
		for (int i = 0; i < ClientList.size(); ++i)
		{
			memset(id, 0, 5);
			memset(port, 0, 128);
			sockaddr_in tmpaddr;
			int tmplen = sizeof(sockaddr_in);
			getpeername(ClientList[i].socket, (sockaddr*)&tmpaddr, &tmplen);
			_itoa(i, id, 10);
			int P = ntohs( tmpaddr.sin_port );
			_itoa(P, port, 10);
			strcat(SendBuffer, "ID:");
			strcat(SendBuffer, id);
			strcat(SendBuffer, "&&IP:");
			strcat(SendBuffer, ClientList[i].IP);
			strcat(SendBuffer, "&&Port");
			strcat(SendBuffer, port);
			strcat(SendBuffer, "\n");
		}
		strcat(SendBuffer,"$@");
		//printf("List info is %s\n", SendBuffer);
		if (send(cl.socket, SendBuffer, strlen(SendBuffer), 0) == SOCKET_ERROR) {
			printf("send list error with error code: %d\n", WSAGetLastError());
		}
		else
			printf("send list successfully\n");
		break;
	case 4:
		strcat(SendBuffer, "M");
		strcat(SendBuffer, "Source ID:");
		strcat(SendBuffer, sourceid);
		strcat(SendBuffer, "\n");
		for (i = 1; !(BUFFER[i] == '&'&&BUFFER[i + 1] == '&'); ++i) {
			desid[i - 1] = BUFFER[i];
		}
		//strcat(SendBuffer, desid);
		strcat(SendBuffer, BUFFER+i+2);
		strcat(SendBuffer, "$@");
		destination = atoi(desid);
		printf("Client %d is asking for send message\nThe destination Client is %d\n", ID,destination);
		if (destination > ClientList.size()||destination<0) {
			printf("The destionation is out of range\n");
			sendflag = false;
			error_code = 2;
		}
		else {
			cld = ClientList[destination];
			//向目的client发送消息
			if (cld.state == 0) {
				printf("The destionation has closed\n");
				sendflag = false;
				error_code = 3;
			}
			else if (destination==ID) {
				printf("The source id equals to the destination id\n");
				sendflag = false;
				error_code = 4;
			}
			else
			{
				if (send(cld.socket, SendBuffer, strlen(SendBuffer), 0) == SOCKET_ERROR) {
					printf("send message to destination error with error code: %d\n", WSAGetLastError());
					sendflag = false;
					error_code = WSAGetLastError();
				}
				else
					printf("send message to destination successfully\n");
			}
		}
		//向来源client发送消息
		memset(SendBuffer, 0, LEN);
		strcat(SendBuffer, "M");
		strcat(SendBuffer, "Your ID:");
		strcat(SendBuffer, sourceid);
		strcat(SendBuffer, "\n");
		if (sendflag == true)
			strcat(SendBuffer, "Success$@");
		else {
			strcat(SendBuffer, "Failed with error code:");
			_itoa(error_code, Error_code, 10);
			strcat(SendBuffer, Error_code);
			if (error_code == 2) {
				strcat(SendBuffer, " The destionation is out of range");
			}
			else if (error_code == 3) {
				strcat(SendBuffer, " The destionation has closed");
			}
			else if (error_code == 4) {
				strcat(SendBuffer, " The source id equals to the destination id");
			}
			strcat(SendBuffer, "$@");
		}
		if (send(cl.socket, SendBuffer, strlen(SendBuffer), 0) == SOCKET_ERROR) {
			printf("send message to source error with error code: %d\n", WSAGetLastError());
		}
		else
			printf("send message to source successfully\n");
		break;
	//no case 5
	default:
		printf("Client %d send a message to service, following is the context: \n", ID);
		printf("%s\n", BUFFER[1]);
		break;
	}
}
void* Client_thread(void *ID) {
	int id = (int)ID;
	Client cl = ClientList[id];
	char BUFFER[LEN];
	char tmp[LEN];
	int Length;
	memset(BUFFER, 0, LEN);
	strcat(BUFFER, "RHello$@\0");
	send(cl.socket, BUFFER,strlen(BUFFER), 0);
	while (true) {
		memset(BUFFER, 0, LEN);
		memset(tmp, 0, LEN);
		if ((Length = recv(cl.socket, tmp, LEN, 0)) > 0) {
			printf("receive a message from client %d\n", id);
			int i;
			for (i = 0; i < Length - 1 && !(tmp[i] == '$'&&tmp[i + 1] == '@'); ++i)
				BUFFER[i] = tmp[i];
			BUFFER[i] = '\0';
			if (BUFFER[0] == 'C') {
				printf("Get a close requirement from client%d\n",id);
				ClientList[id].state = 0;
				pthread_exit(NULL);
			}
			else{
				Response(BUFFER,id);
			}
			
		}
		else
		{
			printf("Close connect with client%d\n",id);
			ClientList[id].state = 0;
			pthread_exit(NULL);
		}
	}
}
int main(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET sListen, sServer; //侦听套接字，连接套接字
	struct sockaddr_in saServer, saClient;//地址信息
	int ret;
	//WinSock初始化：
	printf("Start Server\n");
	wVersionRequested = MAKEWORD(2, 2);//希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return -1;
	}
	//确认WinSock DLL支持版本2.2：
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return -1;
	}

	//创建socket，使用TCP协议：
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return -1;
	}

	//构建本地地址信息：
	saServer.sin_family = AF_INET;//地址家族
	saServer.sin_port = htons(SERVER_PORT);//注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = INADDR_ANY;//使用INADDR_ANY指示任意地址

	//绑定：
	ret = bind(sListen, (LPSOCKADDR)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("bind() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);//关闭套接字
		WSACleanup();
		return -1;
	}
	else {
		printf("successfully bind the port\n");
	}

	//监听
	if (listen(sListen, 20) == SOCKET_ERROR)
	{
		printf("Socket Listener Error: Error Code %d\n", WSAGetLastError());
		return -1;
	}
	else
	{
		printf("listen a client\n");
		pthread_create(&QuitThread, NULL, Quit_thread, (void*)sListen);
	}
	int len = sizeof(sockaddr);
	while ((sServer = accept(sListen, (sockaddr*)&saClient, &len)) != INVALID_SOCKET) {
		//必须要考虑到一个客户端断开之后再重新连接的问题
		//如果经常发生这个情况，然后不停在vector后面push但是不释放原来的那个的话，会出问题
		//用erase的话，存在性能问题，所以需要先做个map去做映射，看看同样的ip：port的节点是否存在
		//测试版本就先不上线这个功能了，跑起来跑通才是最重要的

		//2019/10/15注：表示不需要考虑这个问题，因为，必然是重新向系统申请一个端口的
		//然后原来那个弃用
		printf("A new client is connected, the id is %d\n", ClientList.size());
		struct Client CL = Client();
		CL.ID = ClientList.size();
		CL.socket = sServer;
		CL.state = 1;
		strcpy(CL.IP, inet_ntoa(saClient.sin_addr));
		ClientList.push_back(CL);
		pthread_create(&CL.ClientThread, NULL, Client_thread, (void*)CL.ID);
	}
	printf("Close server\n");
	closesocket(sListen);
	WSACleanup();
	system("pause");
	return 0;
}