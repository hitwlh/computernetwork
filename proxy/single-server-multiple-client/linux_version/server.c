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
SOCKET GLOBAL[100];
void *handle_a_connect(void *aa)
{
	SOCKET ClientSocket = *(SOCKET*)aa;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	do {
		iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if (iResult > 0) {
			printf("Server received: %s\n", recvbuf);
			int iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", errno);
				close(ClientSocket);
				return NULL;
			}
			printf("Server send: %s\n", recvbuf);
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
			iResult = shutdown(ClientSocket, SHUT_RDWR);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed with error: %d\n", errno);
				close(ClientSocket);
				return NULL;
			}
			close(ClientSocket);
		}
		else {
			printf("recv failed with error: %d\n", errno);
			close(ClientSocket);
			return NULL;
		}
	} while (iResult > 0);
	return 0;
}
int main(int argc, char **argv)
{
	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		return 1;
	}
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", errno);
		freeaddrinfo(result);
		return 1;
	}
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", errno);
		freeaddrinfo(result);
		close(ListenSocket);
		return 1;
	}
	freeaddrinfo(result);
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", errno);
		close(ListenSocket);
		while (1);
		return 1;
	}
	int i = 0;
	pthread_t  id;
	while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		i = (i + 1) % 100;
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", errno);
			close(ListenSocket);
			return 1;
		}
		GLOBAL[i] = ClientSocket;
		pthread_create(&id, NULL, handle_a_connect, &GLOBAL[i]);
	}
	close(ListenSocket);
	return 0;
}