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
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char *server;
	if (argc != 2)	server = "127.0.0.1";
	else	server = argv[1];
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		return 1;
	}
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", errno);
			return 1;
		}
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("socket failed with error\n");
			close(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		printf("i'm going to break\n");
		break;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		return 1;
	}
	scanf(" %s", sendbuf);
	printf("strlen(sendbuf) is %d, sendbuf is %s\n", strlen(sendbuf), sendbuf);
	while (strcmp("!", sendbuf) != 0) {
		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", errno);
			close(ConnectSocket);
			return 1;
		}
		printf("client Sent: %s\n", sendbuf);
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Client received: %s\n", recvbuf);
		scanf("%s", sendbuf);
		//sendbuf[strlen(sendbuf)] = '\0';
	}
	iResult = shutdown(ConnectSocket, SHUT_RDWR);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", errno);
		close(ConnectSocket);
		return 1;
	}
	close(ConnectSocket);
	return 0;
}
