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
#define IP_LEN 128			//because the size of IPv6 is 128����ȻӦ�ò������Ÿ������ǿռ�����
#define LEN 1460		//����CSMA/CDЭ���һ֡�����1518��ȥ20B��TCP��ͷ���ټ�ȥ20�ֽ�IPv4��ͷ�����ܹ�1478��B
//Ȼ���ȥ������·�㿪ͷԴ��Ŀ��MAC��ַ6B��Ȼ��2BЭ�����ͣ�4B��CRC���ܹ�ʣ��1460B
//��Ȼ��Ҳ��֪���ǲ��ǰ���CSMA/CDЭ��д�ģ���Ҳ��֪���Լ���û���
#define DELAY 2000		//���峬ʱ��ʱ�䣬����2000����û����Ӧ��������ʱ
						//Ȼ�������ôŪ��

using namespace std;
/*
	���ĵĶ��壺
	�����ַ��ǡ�%@����ʾһ�����ĵĽ���
	�����ܹ����౨������
	һ��������ʱ�䣬�ԡ�T����ͷ������1
	һ��������name���ԡ�N����ͷ������2
	����һ����������У��ԡ�L����ͷ������3
	һ���Ƿ�����Ϣ�ı��ģ��ԡ�M����ͷ������4��������Ҫ��Ŀ��client��message֮������'$$'��Ϊ�ָ�
	�����һ������������͹ر����ӵ���Ϣ���ԡ�C����ͷ������5
	�����Ŀ�ͷ��ʽ������6
*/
//���������һ����Ϣ���У��û���������֤���̺߳����̶߳���Ϣ���еķ���
struct msg {
	int type;		//��Ϣ����
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
		printf("%s\n", msgbuff);	//��ʱ�ȿ���ȫ������ӡ����
									//����ʵ���ϣ�Ӧ���Ǹ��ݲ�ͬ�ķ���message�����ݴ�ӡ��ͬ��
	}
};
pthread_mutex_t Queue_mutex;
queue<struct msg> Client_msgQueue;

//����������һ���ͻ��˵���Ϣ�б�
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
