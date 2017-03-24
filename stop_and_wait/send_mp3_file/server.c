#include <iostream>
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
#include <pthread.h>
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
	cout << "sizeof(int): " << sizeof(int) << endl;
	cout << "DEFAULT_BUFLEN: " << DEFAULT_BUFLEN << endl;
	//const char *destfile = "/SPclasses/yul.mp3";
	const char *destfile = "/SPclasses/a.pdf";
	std::ofstream fout(destfile, std::ios::binary | std::ios::app);  
	SOCKET ListenSocket = INVALID_SOCKET;
	int iSendResult;
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
	ti.tv_sec = 2;
	ti.tv_usec = 0;
	setsockopt(ListenSocket,SOL_SOCKET,SO_RCVTIMEO,&ti,sizeof(ti));
	int send_result;
	int my_want = 0;
	int write_f = 0;
	cout << "my_want is: " << my_want << endl;
	while (1) {
		//cout << "hello world1\n";
		int ret = recvfrom(ListenSocket, recvbuf, DEFAULT_BUFLEN, 0, (struct sockaddr *)&client, &nAddrLen);
		printf("ret is %d\n", ret);
		//cout << "hello world2\n";
		if (ret > 0)
		{
			int num = *((int *)(recvbuf + ret - sizeof(int)));
			//cout << "receiving: " << num << endl;
			cout << "num is: " << num << " my_want is " << my_want << endl;
			if(num == my_want){
				write_f += ret - sizeof(int);
				fout.write(recvbuf, ret - sizeof(int));
				my_want ++;
				void *use = &num;
				memcpy(sendbuf, (char *)use, sizeof(int));
				send_result = sendto(ListenSocket, sendbuf, sizeof(int), 0, (struct sockaddr *)&client, nAddrLen);
				if(send_result == sizeof(int))
					cout << "sending: " << num << endl;
				else
					cout << "send error" << endl;
			}
			else if(my_want > 0){
				num = my_want - 1;
				void *use = &num;
				memcpy(sendbuf, (char *)use, sizeof(int));
				send_result = sendto(ListenSocket, sendbuf, sizeof(int), 0, (struct sockaddr *)&client, nAddrLen);
				if(send_result == sizeof(int))
					cout << "sending: " << num << endl;
				else
					cout << "send error" << endl;
			}
			//cout << "hello world3\n";
			if (fout.bad()){
				cout << "write " << write_f << "Bytes" << endl;
				cout << "file error\n";
				break;
			}	
		}
		else{
			fout.close();
			cout << "write " << write_f << "Bytes" << endl;
			cout << "receive failed\n";
		}
	}
	close(ListenSocket);
	return 0;
}