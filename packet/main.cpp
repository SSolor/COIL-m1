#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")


#include "packet.h"
#include <stdio.h>

int main(void) {

	PktDef p;
	p.SetCmd(RESPONSE);
	p.SetPktCount(5);
	//response has no body
	char* x = p.GenPacket();
	printf("our packet: %d\n", &x);
	for (int i = 0;i < p.GetLength();i++) {
		printf("%d", x[i]);
	}
	printf("\n");


	WSADATA wsaData;
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		return -1;
	}

	//initializes socket. SOCK_STREAM: TCP
	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ClientSocket == INVALID_SOCKET) {
		WSACleanup();
		return -1;
	}

	//Connect socket to specified server
	sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;						//Address family type itnernet
	SvrAddr.sin_port = htons(29500);					//port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr("10.172.41.150");

	char recv[150];
	int SaddrLen = sizeof(SvrAddr); //needed by rcvfrom for some reason


	sendto(ClientSocket, x, p.GetLength(), 0, (sockaddr*)&SvrAddr, sizeof(SvrAddr));
	int a=recvfrom(ClientSocket, recv, 150, 0, (struct sockaddr*)&SvrAddr, &SaddrLen);
	printf("%d\n", a);
	for (int i = 0; i < a;i++) {
		printf("%d",recv[i]);
	}

	PktDef r(recv);
	printf("\nwe got: %d, %d, %d, %s\n", r.GetAck(), r.GetCmd(), r.GetPktCount(), r.GetBodyData());

	//	MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);
	//soc.SendData(x, sizeof(x));
	
	//char rc[150];
	//int recvied = soc.GetData(rc);

	return 0;
}