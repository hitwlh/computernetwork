#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>

typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"




SOCKET GLOBAL[100];
void show_socket_information(SOCKET ClientSocket)
{
	struct sockaddr_in sockAddr;
	int iLen = sizeof(sockAddr);
	//»ñÈ¡local ip and port
	memset(&sockAddr, 0, sizeof(sockAddr));
	//ZeroMemory(&sockAddr, sizeof(sockAddr));
	getpeername(ClientSocket, (struct sockaddr *)&sockAddr, &iLen);//µÃµ½Ô¶³ÌIPµØÖ·ºÍ¶Ë¿ÚºÅ
	char *nSourceIP = inet_ntoa(sockAddr.sin_addr);
	int nSourcePort = ntohs(sockAddr.sin_port);

	//»ñÈ¡remote ip and port
	memset(&sockAddr, 0, sizeof(sockAddr));
	getsockname(ClientSocket, (struct sockaddr *)&sockAddr, &iLen);//µÃµ½±¾µØµÄIPµØÖ·ºÍ¶Ë¿ÚºÅ
	char *nDestIP = inet_ntoa(sockAddr.sin_addr);//IP
	int nDestPort = ntohs(sockAddr.sin_port);
	printf("source- %s:%d, dest- %s:%d\n", nSourceIP, nSourcePort, nDestIP, nDestPort);

}
void *handle_a_connect(void *aa)
{
	SOCKET ClientSocket = *(SOCKET*)aa;
	printf("ClientSocket information: \n");
	show_socket_information(ClientSocket);
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	do {
		iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if (iResult > 0) {
			printf("Server received: %s\n", recvbuf);
			// Echo the buffer back to the sender
			int iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", errno);
				close(ClientSocket);
				 
				while (1);
				return NULL;
			}
			printf("Server send: %s\n", recvbuf);
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
			//iResult = shutdown(ClientSocket, SHUT_RDWR);
			close(ClientSocket);
		}
		else {
			printf("recv failed with error: %d\n", errno);
			close(ClientSocket);
			 
			while (1);
			return NULL;
		}
	} while (iResult > 0);
	return 0;
}
int main(void)
{
	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	//int recvbuflen = DEFAULT_BUFLEN;
	// Initialize Winsock
	//iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		while (1);
		return 1;
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		 
		while (1);
		return 1;
	}
	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", errno);
		freeaddrinfo(result);
		 
		while (1);
		return 1;
	}
	// Setup the TCP listening socket
	printf("before bind, listen information: \n");
	show_socket_information(ListenSocket);
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	printf("after bind, listen information: \n" );
	show_socket_information(ListenSocket);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", errno);
		freeaddrinfo(result);
		close(ListenSocket);
		 
		while (1);
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
	// Accept a client socket
	int i = 0;
	while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		i = (i + 1) % 100;
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", errno);
			close(ListenSocket);
			 
			while (1);
			return 1;
		}
		GLOBAL[i] = ClientSocket;
		pthread_t  id;
		pthread_create(&id, NULL, handle_a_connect, &GLOBAL[i]);
		//HANDLE hand = CreateThread(NULL, 0, handle_a_connect, &GLOBAL[i], 0, NULL);
	}
	// No longer need server socket
	close(ListenSocket);

	return 0;
}


