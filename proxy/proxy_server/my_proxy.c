#undef UNICODE
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#define DEFAULT_PORT "27015"
//#define INTERNET_PORT "27014"
#define DEFAULT_BUFLEN 20480
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
SOCKET GLOBAL[1000];
//really important!!

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

DWORD WINAPI proxy(LPVOID aa)
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
		closesocket(ClientSocket);
		printf("fuck\n");
		return 0;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(ss[0], ss[1], &hints, &result) != 0) {
		printf("getaddrinfo failed with error\n");
		WSACleanup();
		while (1);
		return 1;
	}
	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		cout << "socket failed with error :" << WSAGetLastError() << endl;
		WSACleanup();
		while (true);
		return 1;
	}
	if (connect(ConnectSocket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
		printf("socket failed with error\n");
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		while (1);
		return 1;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		while (1);
		return 1;
	}
	iResult = send(ConnectSocket, recvbuf, iResult, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		while (1);
		return 1;
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
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			while (1);
			return 1;
		}
		//printf("Sent to my_computer: ==============================================\n%s", recv_internet);
		//iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
	} while (iResult > 0);
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		while (1);
		return 1;
	}
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		while (1);
		return 1;
	}
	return 0;
}
int main(void)
{
	WSADATA wsaData;
	int iResult;
	string s_recv_array[DEFAULT_BUFLEN];
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iSendResult;
	//char recv_internet[DEFAULT_BUFLEN];
	//int recvbuflen = DEFAULT_BUFLEN;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		while (1);
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		while (1);
		return 1;
	}
	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		while (1);
		return 1;
	}
	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		while (1);
		return 1;
	}
	freeaddrinfo(result);
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		while (1);
		return 1;
	}
	// Receive until the peer shuts down the connection
	int i = 0;
	while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			while (1);
			return 1;
		}
		i = (i + 1) % 1000;
		GLOBAL[i] = ClientSocket;
		HANDLE hand = CreateThread(NULL, 0, proxy, &GLOBAL[i], 0, NULL);

	}
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		while (1);
		return 1;
	}
	// cleanup
	printf("hello world\n");
	closesocket(ClientSocket);
	WSACleanup();
	while (1);
	return 0;
}