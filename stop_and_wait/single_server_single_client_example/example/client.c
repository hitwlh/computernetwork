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
	SOCKET ConnectSocket = INVALID_SOCKET;
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	char *server;
	if (argc != 2)	server = "127.0.0.1";
	else	server = argv[1];
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(27015);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	int len = sizeof(sin);
	ConnectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	scanf(" %s", sendbuf);
	while (strcmp("!", sendbuf) != 0) {
		iResult = sendto(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0, (struct sockaddr *)&sin, len);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", errno);
			close(ConnectSocket);
			return 1;
		}
		printf("client Sent: %s\n", sendbuf);
		iResult = recvfrom(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0, (struct sockaddr *)&sin, &len);
		if (iResult > 0)
			printf("Client received: %s\n", recvbuf);
		scanf(" %s", sendbuf);
		printf("sendbuf is: %s\n",sendbuf);
		//sendbuf[strlen(sendbuf)] = '\0';
	}
	close(ConnectSocket);
	return 0;
}
