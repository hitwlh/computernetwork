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
#define DEFAULT_BUFLEN DEFAULT_DATASIZE+sizeof(int)
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
typedef int SOCKET; 
typedef unsigned char BYTE;
typedef unsigned long DWORD; 
int main(int argc, char **argv)
{
	//const char *sourcefile = "/SPclasses/tmp/yul.mp3";
	const char *sourcefile = "/SPclasses/tmp/a.pdf";
	cout << "sizeof(int): " << sizeof(int) << endl;
	cout << "DEFAULT_BUFLEN: " << DEFAULT_BUFLEN << endl;
	std::ifstream fin(sourcefile, std::ios::binary);
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[5] = "0000";
	int iResult;
	char server[20];
	if (argc != 2)	strcpy(server, "127.0.0.1");
	else	strcpy(server, argv[1]);
	//cout << "hello world\n";
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(27015);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t len = sizeof(sin);
	SOCKET ReceiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int sending = 0;
	fin.read(sendbuf, sizeof(char) * DEFAULT_DATASIZE);
	void *tmp = &sending;
	char *use = (char *)tmp;
	memcpy(sendbuf + fin.gcount(), use, sizeof(int));
	struct timeval ti;   
	ti.tv_sec = 2;
	ti.tv_usec = 0;
	setsockopt(ReceiveSocket,SOL_SOCKET,SO_RCVTIMEO,&ti,sizeof(ti));
	int i = 0;
	int send_flag = 1;
	//cout << "hello world\n";
	int use2 = -1;
	int throw_flag = 1;
	while(!fin.eof() || send_flag == 1){
		//cout << "hello world1\n";
		//if(i++ > 1500000)
		//	break;
		if(send_flag){
			//cout << "hello world2\n";
			//printf("send how much: %d\n", fin.gcount()+sizeof(int));
			iResult = sendto(ReceiveSocket, sendbuf, fin.gcount()+sizeof(int), 0, (struct sockaddr *)&sin, len);
			cout << "sending: " << sending << endl;
			send_flag = 0;
			
		}
		else{
			throw_flag++;
			//cout << "hello world3\n";
			iResult = recvfrom(ReceiveSocket, recvbuf, sizeof(int), 0, (struct sockaddr *)&sin, &len);
			if((throw_flag % 100) != 0){
				//模拟丢包，如果丢了包，use2还是一开始的值，就会重发已经发过的包
				if(iResult == sizeof(int)){
					use2 = *((int *)recvbuf);
					cout << "receiving: " << use2 << endl;
				}
				else{
					cout << "do not receiving anything" << iResult << endl;
					use2 = -1;
				}
			}
			else{
				printf("throw ack %d\n", (iResult == sizeof(int)) ? *((int *)recvbuf) : -1);
			}
			send_flag = 1;
		}
		//cout << "hello world4\n";
		if(use2 == sending){
			sending = (sending+1) % 2;
			strcpy(recvbuf, "0000");
			fin.read(sendbuf, sizeof(char) * DEFAULT_DATASIZE);
			void *tmp = &sending;
			use = (char *)tmp;
			memcpy(sendbuf + fin.gcount(), use, sizeof(int));
			send_flag = 1;
			continue;
		}
	}
	cout << "fin.gcount()+sizeof(int) is:" << sizeof(int) << endl;
	sendto(ReceiveSocket, sendbuf, sizeof(int), 0, (struct sockaddr *)&sin, len);
	fin.close();
	close(ReceiveSocket);
	return 0;
}
