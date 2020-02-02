#include <winsock2.h>
#include <stdio.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#pragma comment(lib,"WS2_32.lib")

using namespace std;

DWORD WINAPI ScanThread(LPVOID port);

int main(int argc, char* argv[])
{
	WSADATA  wsd;
	int port = 0;
	int MAX_PORT;
	clock_t start, end;
	HANDLE handle;
	DWORD dwThreadId;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		printf("WSAStartup failed!\n");
		return 1;
	}
	string str = argv[1];
	MAX_PORT = atoi(str.c_str());
	cout<<MAX_PORT<<endl;
	start = clock();

	do {
		handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScanThread, (LPVOID)port, 0, &dwThreadId);
		port++;
	} while (port < MAX_PORT);
	WaitForSingleObject(handle, INFINITE);
	end = clock();
	int duration = end - start;
	printf("总耗时 %d ms", duration);
	system("pause");
	return 0;
}

DWORD WINAPI ScanThread(LPVOID port)
{
	int Port = (int)(LPVOID)port;
	int retval;
	SOCKET sHost;
	SOCKADDR_IN servAddr;
	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sHost)
	{
		printf("socket failed!\n");
		WSACleanup();
		return -1;
	}
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	servAddr.sin_port = htons(Port);
	retval = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
	if (retval == SOCKET_ERROR) {
		closesocket(sHost);
		return -1;
	}
	printf("%d端口开放！\n", Port);
	closesocket(sHost);
	return 1;
}
