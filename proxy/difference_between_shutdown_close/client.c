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
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
SOCKET GLOBAL[100];
char ss[20] = "abcdefghijklmnopqr";
char recvbuf[20] = "abcdefghijklmnopqr";
void show_socket_information(SOCKET ClientSocket)
{
	struct sockaddr_in sockAddr;
	int iLen = sizeof(sockAddr);
	//»ñÈ¡local ip and port
	memset(&sockAddr, 0, sizeof(sockAddr));
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




void *send_information(void *aa)
{
	SOCKET ConnectSocket = *(SOCKET*)aa;
	int iResult = send(ConnectSocket, ss, 20, 0);
	if(iResult < 0)
		printf("Sending error!\n");
	else
		printf("client Sent: %s\n", ss);
	iResult = recv(ConnectSocket, recvbuf, 20, 0);
	if (iResult > 0)
		printf("Client received: %s\n", recvbuf);


	printf("shutdowning...\n");
	//iResult = shutdown(ConnectSocket, SHUT_RDWR);
	//iResult = shutdown(ConnectSocket, SHUT_RD);
	iResult = shutdown(ConnectSocket, SHUT_WR);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", errno);
		close(ConnectSocket);
		return NULL;
	}

	/*printf("closing...\n");
	close(ConnectSocket);*/
}
int main(int argc, char **argv)
{
	//WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendbuf[DEFAULT_BUFLEN];
	//char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;


	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n%d", argv[0],argc);
		while (1);
		return 1;
	}


	// Initialize Winsock
	//iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		 
		while (1);
		return 1;
	}
	printf("argv[0] is :%s, argv[1] is: %s\n", argv[0], argv[1]);
	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {


		// Create a SOCKET for connecting to server

		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", errno);
			 
			while (1);
			return 1;
		}


		// Connect to server.

		printf("before connect, ConnectSocket information: \n");
		show_socket_information(ConnectSocket);

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		printf("after connect, ConnectSocket information: \n");
		show_socket_information(ConnectSocket);

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
		 
		while (1);
		return 1;
	}


	// Send an initial buffer
	
	int a[20];
	int i;
	for(i = 0;i<20;i++)
		a[i] = i;

	pthread_t idd;
	for(i = 0;i<20;i++){
		GLOBAL[i] = ConnectSocket;
		pthread_create(&idd, NULL, send_information, &GLOBAL[i]);
		printf("i'm here to hello world\n");
		sleep(1);
	}
	printf("i'm here to fuck world\n");
	return 0;
}
