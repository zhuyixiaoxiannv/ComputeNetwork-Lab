#pragma once
#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <pthread.h>
#include <WS2tcpip.h>


#define SERVER_PORT	441
#define HOST_IP "127.0.0.1"
#define IP_LEN 128			//because the size of IPv6 is 128，虽然应该不会用着个，但是空间留着
#define LEN 1460		//按照CSMA/CD协议的一帧最长长度1518减去20B的TCP报头，再减去20字节IPv4的头部，总共1478个B
//然后减去数据链路层开头源和目的MAC地址6B，然后2B协议类型，4B的CRC，总共剩下1460B
//虽然我也不知道是不是按照CSMA/CD协议写的，我也不知道自己算没算错
#define DELAY 2000		//定义超时的时间，超过2000毫秒没有响应就算作超时
						//然而这个怎么弄？

using namespace std;
/*
	报文的定义：
	结束字符是“%@”表示一个报文的结束
	报文总共四类报文类型
	一个是请求时间，以“T”开头，代码1
	一个是请求name，以“N”开头，代码2
	还有一个是请求队列，以“L”开头，代码3
	一个是发送消息的报文，以“M”开头，代码4，另外需要在目的client和message之间增加'$$'作为分隔
	最后是一个向服务器发送关闭连接的消息，以“C”开头，代码5
	其他的开头格式，代码6
*/
//接下来设计一个消息队列，用互斥锁来保证主线程和子线程对消息队列的访问
struct msg {
	int type;		//消息类型
	char msgbuff[1460];
	int length;
public:
	msg(int t, char buff[],int l) {
		type = t;
		length = l;
		for (int i = 0; i < l; ++i) {
			msgbuff[i] = buff[i];
		}
	}
	char* getmsgInfo() { return msgbuff; }
	int gettype() { return type; }
	void setmsg(int t, char buff[],int l) {
		type = t;
		length = l;
		for (int i = 0; i < l; ++i) {
			msgbuff[i] = buff[i];
		}
	}
	void print_msg() {
		printf("####################### received  message #######################\n");
		printf("%s\n", msgbuff);	//暂时先考虑全部都打印出来
									//但是实际上，应该是根据不同的返回message的内容打印不同的
	}
};
pthread_mutex_t Queue_mutex;
queue<struct msg> Client_msgQueue;

//接下来定义一个客户端的信息列表
struct Info_list {
	int ID;
	u_long IP;
	u_short Port;
};
int gettype(char type) {
	switch (type)
	{
	case 'T':
		return 1;
	case 'N':
		return 2;
	case 'L':
		return 3;
	case 'M':
		return 4;
	case 'C':
		return 5;
	default:
		return 6;
	}
}
vector<struct Info_list> Client_list;
#endif // !_CLIENT_H_
