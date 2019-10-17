#pragma once
#ifndef _SERVER_H_
#define _SERVER_H_
#include <WinSock2.h>
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <WS2tcpip.h>
#define SERVER_PORT	441
#define HOST_IP "127.0.0.1"
#define IP_LEN 128
#define LEN 1460

struct Client{
public:
	SOCKET socket;
	int ID;
	int state;
	pthread_t ClientThread;
	char IP[IP_LEN];
	int Port;
};

struct msg {
	int type;
	char msgbuff[1460];
	int length;
public:
	msg(int t, char buff[], int l) {
		type = t;
		length = l;
		for (int i = 0; i < l; ++i) {
			msgbuff[i] = buff[i];
		}
	}
	char* getmsgInfo() { return msgbuff; }
	int gettype() { return type; }
	void setmsg(int t, char buff[], int l) {
		type = t;
		length = l;
		for (int i = 0; i < l; ++i) {
			msgbuff[i] = buff[i];
		}
	}
	void print_msg() {
		printf("%s\n", msgbuff);
	}
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
#endif // !_SERVER_H_

