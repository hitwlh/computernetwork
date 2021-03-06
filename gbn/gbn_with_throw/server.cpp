﻿#include <iostream>
#include <fstream>
using namespace std;

#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#define M 2
#define continue_throw_ack_time (1<<M)-1
#define DEFAULT_PORT "27015"
#define DEFAULT_DATASIZE 256
#define DEFAULT_BUFLEN DEFAULT_DATASIZE+sizeof(int)//DEFAULT_DATASIZE是数据，4是包的编号（int大小），
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
typedef int SOCKET; 
typedef unsigned char BYTE;
typedef unsigned long DWORD; 
int main(int argc, char **argv)
{
	int iResult;
	//int throw_flag = 0;
	cout << "sizeof(int): " << sizeof(int) << endl;
	cout << "DEFAULT_BUFLEN: " << DEFAULT_BUFLEN << endl;
	//const char *destfile = "/SPclasses/yul.mp3";
	const char *destfile = "/SPclasses/a.pdf";
	std::ofstream fout(destfile, std::ios::binary | std::ios::app);  
	SOCKET ListenSocket = INVALID_SOCKET;
	char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[5];
	ListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", errno);
		return 1;
	}
	struct sockaddr_in result;
	result.sin_family = AF_INET;
	result.sin_port = htons(27015);
	result.sin_addr.s_addr = INADDR_ANY;
	iResult = bind(ListenSocket, (struct sockaddr *)&result, sizeof(result));
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", errno);
		close(ListenSocket);
		return 1;
	}
	struct sockaddr_in client;
	socklen_t nAddrLen = sizeof(client); 
	struct timeval ti;   
	ti.tv_sec = 0;
	ti.tv_usec = 1;
	setsockopt(ListenSocket,SOL_SOCKET,SO_RCVTIMEO,&ti,sizeof(ti));
	int my_want = 0, write_f = 0, send_result;
	int number_size = 1 << M;
	int throw_flag = 0;
	//cout << "my_want is: " << my_want << endl;
	int chaoshi = 0;
	int throw_ack_time = continue_throw_ack_time;
	while (chaoshi < 5) {
		//throw_flag++;
		//printf("throw_flag is %d\n", throw_flag);
		//cout << "hello world1\n";
		int ret = recvfrom(ListenSocket, recvbuf, DEFAULT_BUFLEN, 0, (struct sockaddr *)&client, &nAddrLen);
		//printf("ret is %d\n", ret);
		//cout << "hello world2\n";

		if (ret - sizeof(int) == 0){
			cout << "receive failed\n";
			cout << "write " << write_f << "Bytes" << endl;
			break;
		}
		throw_flag ++;
		if (ret > 0 && (throw_flag % 100) != 0)
		{
			chaoshi = 0;
			int num = *((int *)(recvbuf + ret - sizeof(int)));
			//cout << "receiving: " << num << endl;
			//cout << "num is: " << num << " my_want is " << my_want << endl;
			if(num == my_want){
				write_f += ret - sizeof(int);
				//printf("fout.write(recvbuf, %d);", ret - sizeof(int));
				fout.write(recvbuf, ret - sizeof(int));
				my_want = (my_want+1)%number_size;
				void *use = &my_want;
				memcpy(sendbuf, (char *)use, sizeof(int));
				if(throw_flag % 50 != 0 && throw_ack_time == continue_throw_ack_time){
					send_result = sendto(ListenSocket, sendbuf, sizeof(int), 0, (struct sockaddr *)&client, nAddrLen);
					if(send_result == sizeof(int))
						cout << "ack" << my_want << endl;
					else
						cout << "send error" << endl;
				}
				else{
					//cout << "throw ack" << my_want << endl;
					throw_ack_time--;
					if(throw_ack_time == 0)
						throw_ack_time = continue_throw_ack_time;
					cout << "throw ack" << my_want << ", throw_ack_time is: " << throw_ack_time << endl;
				}
			}
			else{
				//printf("num is: %d);\n", num);
				void *use = &my_want;
				memcpy(sendbuf, (char *)use, sizeof(int));
				if(throw_flag % 50 != 0 && throw_ack_time == continue_throw_ack_time){
					send_result = sendto(ListenSocket, sendbuf, sizeof(int), 0, (struct sockaddr *)&client, nAddrLen);
					if(send_result == sizeof(int))
						cout << "ack" << my_want << endl;
					else
						cout << "send error" << endl;
				}
				else{
					throw_ack_time--;
					if(throw_ack_time == 0)
						throw_ack_time = continue_throw_ack_time;
					cout << "throw ack" << my_want << ", throw_ack_time is: " << throw_ack_time << endl;
				}
			}
			//cout << "hello world3\n";
			if (fout.bad()){
				cout << "write " << write_f << "Bytes" << endl;
				cout << "file error\n";
				break;
			}	
		}
		else if(ret > 0){
			printf("throw package %d\n", *((int *)(recvbuf + ret - sizeof(int))));
			chaoshi++;
		}
		else{
			printf("receive NULL\n");
			chaoshi++;
		}
	}
	cout << "continue_throw_ack_time is: " << continue_throw_ack_time << endl;
	close(ListenSocket);
	fout.close();
	return 0;
}