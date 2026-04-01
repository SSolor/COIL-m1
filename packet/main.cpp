//#include <windows.networking.sockets.h>
//#pragma comment(lib, "Ws2_32.lib")

#include "socket.h"
#include "packet.h"

#include <stdio.h>

int main(void) {

	PktDef p;
	p.SetCmd(SLEEP);
	p.SetPktCount(5);

	DriveBody drive;
	drive.Direction = LEFT;
	drive.Duration = 7;
	drive.Power = 0;

	TurnBody turn;
	turn.Direction = RIGHT;
	turn.Duration = 12;

	//p.SetBodyData((char*) & drive, sizeof(drive));
	char* x = p.GenPacket();
	printf("our packet: %d\n", &x);
	for (int i = 0;i < p.GetLength();i++) {
		printf("%d", x[i]);
	}
	printf("\n");


	if(ONWINDOWS){
		WSADATA wsaData;
		if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
			return -1;
		}
	}
	
	/*
	//initializes socket. SOCK_STREAM: TCP
	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET) {
		WSACleanup();
		return -1;
	}

	//Connect socket to specified server
	sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;						//Address family type itnernet
	SvrAddr.sin_port = htons(29000);					//port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr("10.172.41.150");

	if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
		//I was having some issues at this stage so I added this

		closesocket(ClientSocket);
		return -1;
	}

	char recvi[150];
	send(ClientSocket, x, p.GetLength(), 0);
	int a = recv(ClientSocket, recvi, 150, 0);
	//int SaddrLen = sizeof(SvrAddr); //needed by rcvfrom for some reason
	printf("%d\n", a);
	for (int i = 0; i < a;i++) {
		printf("%d,", recvi[i]);
	}

	PktDef r(recvi);
	printf("\nwe got: ack: %d, cmd: %d, count: %d, %s\n", r.GetAck(), r.GetCmd(), r.GetPktCount(), r.GetBodyData());

	printf("--------\n");

	x[0] = 9;
	char revii[150];

	send(ClientSocket, x, p.GetLength(), 0);
	int b = recv(ClientSocket, revii, 150, 0);
	//int SaddrLen = sizeof(SvrAddr); //needed by rcvfrom for some reason
	printf("%d\n", b);
	for (int i = 0; i < b;i++) {
		printf("%d,", revii[i]);
	}
	PktDef rI(revii);
	printf("\nwe got: ack: %d, cmd: %d, count: %d, %s\n", rI.GetAck(), rI.GetCmd(), rI.GetPktCount(), rI.GetBodyData());
	

	//sendto(ClientSocket, x, p.GetLength(), 0, (sockaddr*)&SvrAddr, sizeof(SvrAddr));
	//int a=recvfrom(ClientSocket, recvi, 150, 0, (struct sockaddr*)&SvrAddr, &SaddrLen);
	*/

	
	
		MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
		soc.SetCType(TCP);
		soc.ConnectTCP();

		//soc.ConnectTCP();
		soc.SendData(x, p.GetLength());
		
		char rc[150];
		int recvied = soc.GetData(rc);
		printf("%d\n", recvied);
		for (int i = 0; i < recvied;i++) {
			printf("%d,", rc[i]);
		}

		PktDef r(rc);
		printf("\nwe got: ack: %d, cmd: %d, count: %d, %s\n", r.GetAck(), r.GetCmd(), r.GetPktCount(), r.GetBodyData());


		printf("--------\n");
		soc.DisconnectTCP();
		soc.SetIPAddr("10.172.41.150");
		soc.SetType(SERVER);
		soc.resetSocket();
		soc.ConnectTCP();

		x[0] = 9;
		soc.SendData(x, p.GetLength());
		char rc2[150];
		int forlan = soc.GetData(rc2);
		printf("%d\n", forlan);
		for (int i = 0; i < forlan;i++) {
			printf("%d,", rc2[i]);
		}
		PktDef rfo(rc2);
		printf("\nwe got: ack: %d, cmd: %d, count: %d, %s\n", rfo.GetAck(), rfo.GetCmd(), rfo.GetPktCount(), rfo.GetBodyData());


		
		
	/*
	SocketType stype = CLIENT;
	std::string IP = "10.172.41.150";
	unsigned int port = 29500;
	ConnectionType ctype = UDP;
	unsigned int size= 100;
	char* Buffer;
	int MaxSize;

	if (size > DEFAULT_SIZE) {
		Buffer = new char[size];
		MaxSize = size;
	}
	else {
		Buffer = new char[DEFAULT_SIZE];
		MaxSize = DEFAULT_SIZE;
	}

	SOCKET ConnectionSocket;
	ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ConnectionSocket == INVALID_SOCKET) {
		if (ONWINDOWS)
			WSACleanup();
			return -1;
	}

	struct sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(port);
	SvrAddr.sin_addr.s_addr = inet_addr(IP.c_str());

	char recv[150];
	int AddrLen;

	int res = sendto(ConnectionSocket, x, p.GetLength(), 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
	int recsize = recvfrom(ConnectionSocket, recv, 150, 0, (struct sockaddr*)&SvrAddr, &AddrLen);
	printf("recieved, size: %d\n", recsize);
	*/
	WSACleanup();
	return 0;
}