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
#define DEFAULT_BUFLEN 20480
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
typedef int SOCKET; 
typedef unsigned char BYTE;
typedef unsigned long DWORD; 

int main(int argc, char **argv)
{
	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
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
	int nAddrLen = sizeof(client); 
	while (1) {
		int ret = recvfrom(ListenSocket, recvbuf, DEFAULT_BUFLEN, 0, (struct sockaddr *)&client, &nAddrLen);
		if (ret > 0)
		{
			printf("connecting：%s \r\n", inet_ntoa(client.sin_addr));
			printf("receive: %s\n",recvbuf);
			sendto(ListenSocket, recvbuf, strlen(recvbuf)+1, 0, (struct sockaddr *)&client, nAddrLen);
			printf("send：%s\n", recvbuf);
		}
	}
	close(client);
	return 0;
}