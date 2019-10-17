#include "Client.h"
#pragma comment(lib,"pthreadVC2.lib")
#pragma comment(lib,"WS2_32.lib")
int state;				//state=1��ʾ�������ӣ�state=0��ʾ���ӹر�
int get_list;
pthread_t subthread;
WSADATA lpWSAData;
SOCKET sListen;
sockaddr_in sServer;
int Screen(bool show) {
	//show some information,and scanf the choose,and return the choose
	//actually I think, it should be a vector of sub thread
	//and when the sub thread number equals to zero
	//
	char Choose[65535];
	int choose=0;
	//clear screen
	//system("cls");
	//but I think cls is not necessary,otherwise we need to 
	if (show == true) {
		printf("#################################################################\n");
		printf("#           Thank you for using this system                     #\n");
		printf("#           please choose from the following choices            #\n");
		printf("#                                                               #\n");
		printf("#           1:connect                                           #\n");
		printf("#           2:close                                             #\n");		//in order to be the same with the pre one
		printf("#           3:get time info                                     #\n");
		printf("#           4:get name info                                     #\n");
		printf("#           5:get client list                                   #\n");
		printf("#           6:sent massage                                      #\n");
		printf("#           7:exit                                              #\n");
		printf("#                                                               #\n");
		printf("#################################################################\n");
	}
	printf("*-----------------------------system----------------------------*\n");
	printf("please enter your choose:\n");
	scanf("%s", &Choose);
	printf("*-----------------------------system----------------------------*\n");
	choose = atoi(Choose);
	printf("Your choice is: %d ", choose);
	switch (choose) {
	case 1:
		printf(": connect\n");
		break;
	case 2:
		printf(": close\n");
		break;
	case 3:
		printf(": get time info\n");
		break;
	case 4:
		printf(": get name info\n");
		break;
	case 5:
		printf(": get client list\n");
		break;
	case 6:
		printf(": sent massage\n");
		break;
	case 7:
		printf(": exit\n");
		break;
	default:
		printf("\nYour choice is illegal,please choose again\n");
		break;
	}
	return choose;
}
void *Listener(void *arg) {
	char BUFFER[LEN];	
	char tmp[LEN];
	int Length;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	while (true) {
		pthread_testcancel();
		memset(BUFFER, 0, LEN);
		memset(tmp, 0, LEN);
		if ((Length = recv(sListen, tmp, LEN, 0)) > 0) {
			int i;
			int type = gettype(tmp[0]);
			for (i = 1; i < Length  &&!(tmp[i] == '$'&&tmp[i + 1] == '@'); ++i)	
				BUFFER[i-1] = tmp[i];
			BUFFER[i] = '\0';
			struct msg m = msg(type,BUFFER,i);
			//printf("REC : \n%s\n", BUFFER);
			pthread_mutex_lock(&Queue_mutex);{
				Client_msgQueue.push(m);		//����һ����Ϣ
			}
			pthread_mutex_unlock(&Queue_mutex);
		}
		else		//����˵�������˵�����Ĺر������ӣ�����������������Ȼ����������������Ҳ�ǻᷢ����
		{
			printf("*-----------------------------system----------------------------*\n");
			printf("the connect has been canceled\n");
			state = 0;
			pthread_exit(NULL);
		}
	}
}

int Connect() {
	printf("*-----------------------------system----------------------------*\n");
	printf("Start connect\n");
	int ret = connect(sListen, (struct sockaddr*)&sServer, sizeof(sServer));
	if (ret == SOCKET_ERROR){
		printf("*-----------------------------system----------------------------*\n");
		printf("connect fail\n");
		return -1;
	}
	else
	{
		printf("*-----------------------------system----------------------------*\n");
		printf("connect success\n");
		state = 1;
		if (pthread_create(&subthread, NULL, Listener, NULL)==0) {
			printf("*-----------------------------system----------------------------*\n");
			printf("sub_thread create success!\n");
		}
		else{
			printf("*-----------------------------system----------------------------*\n");
			printf("sub_thread create fail\n");
			return -1;
		}
		return 1;
	}
}
void Ask(char ch) {
	char ask[LEN];
	memset(ask, 0, LEN);
	ask[0] = ch;
	ask[1] = '$';
	ask[2] = '@';
	send(sListen, ask, LEN, 0);
}
void Close() {
	state = 0;
	Ask('C');
	pthread_cancel(subthread);
		//ע����Ҫ�����̵߳������Ӷ�thread cancel�ļ��
		//��Ϊpthread_cancel ����ֻ�������̷߳�����һ���ر��̵߳���Ϣ������û��ǿ�ƹر�
		//�����߳���Ȼ����while������ʵ������������
		//��ʱ�ܹ������������ȵ���pthread_setcanceltype ����ȡ��״̬Ϊ����ȡ��
		//Ȼ������pthread_testcancel
	closesocket(sListen);
	printf("*-----------------------------system----------------------------*\n");
	printf("Close connect success\n");
}

void SendMassage() {
	char ask[LEN];
	char message[1200];
	char idchar[5];
	int id;
	memset(ask, 0, LEN);
	ask[0] = 'M';
	printf("*-----------------------------system----------------------------*\n");
	printf("please choose the Client number:\n");
	scanf("%d", &id);
	_itoa(id,idchar,10);
	printf("*-----------------------------system----------------------------*\n");
	printf("please choose the message to send(no more than 1200):\n");
	scanf("%s", &message);
	strcat(ask, idchar);
	strcat(ask, "&&");
	//��Ϊ��Ҫ����Ŀ�ĵ�client�ͱ���message�ķָ�
	//������Ϊstrcat֮�󣬵õ���charӦ���ǳ��Ȳ��̶��ģ�������Ҫ���зָ�
	strcat(ask, message);
	strcat(ask, "$@");
	send(sListen, ask, LEN, 0);
}
void lock_print(int msgtype) {
	//ǧ��ǧ��Ҫ��������ʹ��while����ʣ�µ�ȫ����ӡ����
	//whileҪ�������棨��Ҫ������ô֪���ģ�
	bool flag=false;
	while (!flag)
	{
		pthread_mutex_lock(&Queue_mutex);
		if (Client_msgQueue.size() != 0) {
			if (Client_msgQueue.front().gettype() == msgtype)	//������յ���Ϣ֮ǰ���кܶ���Ϣ����Ҫ�����Ǵ�ӡ����
			{
				flag = true;
			}
			Client_msgQueue.front().print_msg();
			Client_msgQueue.pop();
		}
		pthread_mutex_unlock(&Queue_mutex);
	}
}
int main(void) {
	state = 0;
	get_list = 0;
	int ret;
	int connectRet;
	int choose;
	char IP[128];
	u_short port;
	//��ʼ��
	printf("*-----------------------------system----------------------------*\n");
	printf("Start Client\n");
	pthread_mutex_init(&Queue_mutex, NULL);
	ret = WSAStartup(MAKEWORD(2, 2), &lpWSAData);
	if (ret != 0){
		printf("*-----------------------------system----------------------------*\n");
		printf("WSAStartup failed!\n");
		return -1;
	}
	if (LOBYTE(lpWSAData.wVersion) != 2 || HIBYTE(lpWSAData.wVersion) != 2){
		WSACleanup();
		printf("*-----------------------------system----------------------------*\n");
		printf("Invalid Winsock version!\n");
		return -1;
	}
	choose = Screen(true);
	while (true) {
		switch (choose) {
		case 1:
			if (state == 1) {
				printf("*-----------------------------system----------------------------*\n");
				printf("This Client has been connected\n");
				break;
			}
			sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sListen == INVALID_SOCKET)
			{
				WSACleanup();
				printf("*-----------------------------system----------------------------*\n");
				printf("socket() failed!\n");
				return -1;
			}
			printf("*-----------------------------system----------------------------*\n");
			printf("Please enter the targe IP:\n");
			scanf("%s", &IP);
			printf("*-----------------------------system----------------------------*\n");
			printf("Please enter the target Port:\n");
			scanf("%d", &port);
			sServer.sin_family = AF_INET;
			sServer.sin_port = htons(port);
			sServer.sin_addr.S_un.S_addr = inet_addr(IP);
			
			connectRet =Connect();
			if (connectRet == -1)
				return -1;
			break;
		case 2:
			if(state==1)
				Close();
			break;
		case 3:
			if (state == 0) {
				printf("*-----------------------------system----------------------------*\n");
				printf("Please connect first\n");
			}
			else
			{
				Ask('T');
				lock_print(1);
			}
			break;
		case 4:
			if (state == 0) {
				printf("*-----------------------------system----------------------------*\n");
				printf("Please connect first\n");
			}	
			else
			{
				Ask('N');
				lock_print(2);
			}
			break;
		case 5:
			if (state == 0) {
				printf("*-----------------------------system----------------------------*\n");
				printf("Please connect first\n");
			}
			else
			{
				Ask('L');
				lock_print(3);
				get_list = 1;
			}
			break;
		case 6:
			if (state == 0) {
				printf("*-----------------------------system----------------------------*\n");
				printf("Please connect first\n");
			}	
			else
			{
				if (get_list == 0)
				{
					printf("*-----------------------------system----------------------------*\n");
					printf("Please get client list first\n");
				}
				else
				{
					SendMassage();
					lock_print(4);
				}
			}
			break;
		case 7:
			if(state==1)
				Close();
			WSACleanup();
			return 0;		//����break;
		default:
			break;
		}
		//������ͻ���ѡ��Ȼ��Ѷ��ָʾ��Ϣһ������ӡ����������еĻ�
		pthread_mutex_lock(&Queue_mutex);
		while (Client_msgQueue.size() != 0)
		{
			Client_msgQueue.front().print_msg();
			Client_msgQueue.pop();
		}
		pthread_mutex_unlock(&Queue_mutex);
		choose = Screen(false);
	}
	return 0;
}
