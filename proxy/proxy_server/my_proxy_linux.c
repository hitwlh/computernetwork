#undef UNICODE
///#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <string>
#include <string.h>
//#include <iostream>
#include <malloc.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>///#include <winsock2.h>
///#include <ws2tcpip.h>
///#include <windows.h>
///#include <iphlpapi.h>

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



//#pragma comment(lib, "Ws2_32.lib")
//using namespace std;
SOCKET GLOBAL[1000];

char ** ParseHttpRequest(char * SourceBuf, int DataLen, int *a)
{
	char **ss = (char **)malloc(2 * sizeof(char *));
	char * HttpHead = "http://";
	char * FirstLocation = NULL;
	char * LastLocation = NULL;
	char * PortLocation = NULL;
	char  ServerName[DEFAULT_BUFLEN];
	char  *PortString = NULL;
	int   NameLen;
	struct hostent * pHost;
	//struct sockaddr_in * pServer = (struct sockaddr_in *)ServerAddr;
	FirstLocation = strstr(SourceBuf, HttpHead) + strlen(HttpHead);
	printf("FirstLocation: %s\n", FirstLocation);
	LastLocation = strstr(FirstLocation, "/");
	memset(ServerName, 0, DEFAULT_BUFLEN);
	memcpy(ServerName, FirstLocation, LastLocation - FirstLocation);
	PortLocation = strstr(ServerName, ":");
	if (PortLocation != NULL)
	{
		NameLen = PortLocation - ServerName - 1;
		memset(PortString, 0, 10);
		memcpy(PortString, PortLocation + 1, NameLen);
		*PortLocation = 0;
	}
	else
	{
		PortString = "80";
	}
	if (NameLen > DEFAULT_BUFLEN)
	{
		printf("·þÎñÆ÷Ãû×ÖÌ«³¤\n");
		*a = -1;
	}
	ss[0] = ServerName;
	ss[1] = PortString;
	return ss;
	//return 0;
}

void *proxy(void *aa)
{
	char recv_internet[DEFAULT_BUFLEN];
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	SOCKET  ClientSocket = *(SOCKET*)aa;
	char recvbuf[DEFAULT_BUFLEN];
	memset(recvbuf, 0, DEFAULT_BUFLEN);
	int iResult;
	int as = 0;
	iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
	recvbuf[iResult] = '\0';

	if (iResult > 0) {
		printf("received from my computer:==============================================\n");
	}
	SOCKET ConnectSocket;
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	char ** ss = ParseHttpRequest(recvbuf, iResult, &as);
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("ss[0] = %s, ss[1] = %s\n", ss[0], ss[1]);
	if (as< 0) {
		///closesocket(ClientSocket);
		close(ClientSocket);
		printf("fuck\n");
		return NULL;
	}
	///ZeroMemory(&hints, sizeof(hints));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(ss[0], ss[1], &hints, &result) != 0) {
		printf("getaddrinfo failed with error\n");
		
		while (1);
		return NULL;
	}
	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		//cout << "socket failed with error :" << errno << endl;
		printf("socket failed with error : %d\n", errno);
		while (1);
		return NULL;
	}
	if (connect(ConnectSocket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
		printf("socket failed with error\n");
		///closesocket(ConnectSocket);
		close(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		while (1);
		return NULL;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		
		while (1);
		return NULL;
	}
	iResult = send(ConnectSocket, recvbuf, iResult, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", errno);
		///closesocket(ConnectSocket);
		close(ConnectSocket);
		///
		while (1);
		return NULL;
	}
	printf("Sent to internet:==============================================\n%s", recvbuf);

	do {
		if (iResult <= 0)
			break;
		iResult = recv(ConnectSocket, recv_internet, DEFAULT_BUFLEN, 0);
		//recv_internet[iResult] = '\0';
		//printf("receive from internet:==============================================\n%s", recv_internet);
		int iSendResult = send(ClientSocket, recv_internet, iResult, 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", errno);
			///closesocket(ClientSocket);
			close(ClientSocket);
			///
			while (1);
			return NULL;
		}
		//printf("Sent to my_computer: ==============================================\n%s", recv_internet);
		//iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
	} while (iResult > 0);
	iResult = shutdown(ClientSocket, SHUT_RDWR);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", errno);
		///closesocket(ClientSocket);
		close(ClientSocket);
		///
		while (1);
		return NULL;
	}
	iResult = shutdown(ConnectSocket, SHUT_RDWR);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", errno);
		///closesocket(ClientSocket);
		close(ClientSocket);
		///
		while (1);
		return NULL;
	}
	return NULL;
}
int main(void)
{
	printf("hello I'm here1\n");
	///WSADATA wsaData;
	int iResult;
	//string s_recv_array[DEFAULT_BUFLEN];
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iSendResult;
	//char recv_internet[DEFAULT_BUFLEN];
	//int recvbuflen = DEFAULT_BUFLEN;
	// Initialize Winsock
	/**iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		while (1);
		return 1;
	}**/
	///ZeroMemory(&hints, sizeof(hints));
	printf("hello I'm here5\n");
	printf("sizeof(hints) : %d\n",sizeof(hints));
	memset(&hints, 0, sizeof(hints));
	printf("hello I'm here4\n");
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

	printf("hello I'm here3\n");

	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %d\n", errno);
		freeaddrinfo(result);
		
		while (1);
		return 1;
	}
	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", errno);
		freeaddrinfo(result);
		///closesocket(ListenSocket);
		close(ListenSocket);
		///
		while (1);
		return 1;
	}
	freeaddrinfo(result);
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", errno);
		///closesocket(ListenSocket);
		close(ListenSocket);
		///
		while (1);
		return 1;
	}
	// Receive until the peer shuts down the connection
	int i = 0;
	pthread_t  id;




	printf("hello I'm here2\n");
	while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", errno);
			///closesocket(ListenSocket);
			close(ListenSocket);
			///
			while (1);
			return 1;
		}
		i = (i + 1) % 1000;
		GLOBAL[i] = ClientSocket;
		//HANDLE hand = CreateThread(NULL, 0, proxy, &GLOBAL[i], 0, NULL);
		pthread_create(&id, NULL, proxy, &GLOBAL[i]);
		i++;

	}
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SHUT_RDWR);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", errno);
		///closesocket(ClientSocket);
		close(ClientSocket);
		///
		while (1);
		return 1;
	}
	// cleanup
	printf("hello world\n");
	///closesocket(ClientSocket);
	close(ClientSocket);
	///
	while (1);
	return 0;
}