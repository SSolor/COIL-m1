//COIL milestone 2, socket definitions

//we're gonna need an ifdef windows, i'm thinking
//lets just try to get this to work first
#include <winsock2.h>
#include <ws2tcpip.h> 
	//in your most recent example, you used "<windows.networking.sockets.h>", 
	//but I couldn't get sockaddrlen to work wiht that
#pragma comment(lib, "Ws2_32.lib")

//if linux:
/*
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h> //needed for inet_addr()
*/

#include <string>

typedef enum SocketType {
	CLIENT,
	SERVER
};
typedef enum ConnectionType {
	TCP,
	UDP
};
const int DEFAULT_SIZE = 0;

class MySocket {
	char* Buffer;
	SOCKET WelcomeSocket;
	SOCKET ConnectionSocket;
	struct sockaddr_in SvrAddr;
	socklen_t SAddrLen;// ??
	//do we need extras for udp?
	SocketType mySocket;
	std::string IPAddr;
	unsigned int Port;
	ConnectionType connectionType;
	bool bTCPConnect;
	bool TCPserv; //extra for qol
	int MaxSize;
public:
	MySocket(SocketType stype, std::string IP, unsigned int port, ConnectionType ctype, unsigned int size) {
		//basic values
		mySocket = stype;
		IPAddr = IP;
		Port = port;
		connectionType = ctype;

		if (connectionType == TCP && mySocket == SERVER)
			TCPserv = true;
		
		if (size > DEFAULT_SIZE)
			Buffer = new char[size];
		else
			Buffer = new char[DEFAULT_SIZE];


		//socket startup
		
		//ifdef windows
		WSADATA wsaData;
		if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
			exit(EXIT_FAILURE);//if one of you knows how to make this throw an exception that'd be great
		}

		//socket
		if (TCPserv == true) {
			WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (WelcomeSocket == INVALID_SOCKET) {
				//windows
				WSACleanup();
				exit(EXIT_FAILURE);//make this an exception if you could
			}
		}
		else if (connectionType == TCP) {
			ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		else if (connectionType == UDP) {
			ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		}
		if (!TCPserv) {
			if (ConnectionSocket == INVALID_SOCKET) {
				//windows
				WSACleanup();
				exit(EXIT_FAILURE);
			}
		}

		//consolidate info
		SvrAddr.sin_family = AF_INET;
		if (mySocket == SERVER)
			SvrAddr.sin_addr.s_addr = INADDR_ANY;
		SvrAddr.sin_port = htons(Port);
		if (mySocket == CLIENT)
			SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());

		//bind (for servers)
		if (mySocket == SERVER) {
			if (bind((connectionType == TCP) ? WelcomeSocket : ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
				//windows
				closesocket((connectionType == TCP) ? WelcomeSocket : ConnectionSocket);
				WSACleanup();
				//linux
				//close((connectionType == TCP) ? WelcomeSocket : ConnectionSocket);
				exit(EXIT_FAILURE);
			}
		}


	}
	~MySocket() {
		if (Buffer)
			delete[] Buffer;
		//I don't believe anything else is dynamic
	}

	void ConnectTCP() {
		if (connectionType == UDP || bTCPConnect == true) {
			//I think for all these writing to stderr is acceptable? unless there's a cpp way of doing it
			fprintf(stderr, "can't connect like this\n");
			return;
		}
		else {


		}

	}
	void DisconnectTCP() {
		if (connectionType == UDP || bTCPConnect == false) {
			fprintf(stderr, "you can't disconnect right now\n");
			return;
		}
		else{
			//windows
			closesocket(ConnectionSocket);

			//linux
			//close(ConnectionSocket);

			if (mySocket == SERVER) {
				//windows
				closesocket(WelcomeSocket);

				//linux
				//close(WelcomeSocket);
			}

			bTCPConnect == false;

			//windows
			WSACleanup();
			return;
		}
	}
	void SendData(const char* dat, int size) {
		if (connectionType == TCP && bTCPConnect==true) {
			//dont want to call this without having connected
			send(ConnectionSocket, dat, size, 0);
		}
		else if (connectionType == UDP) {
			sendto(ConnectionSocket, dat, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
		}
		return;
	}
	int GetData(char* buf) {

	}
	std::string GetIPAddr() {return IPAddr;}
	void SetIPAddr(std::string newaddr) {
		if (bTCPConnect == true)
			fprintf(stderr, "you cannot change this right now!\n");
		else
			IPAddr = newaddr;
		return;
	}
	void SetPort(unsigned int newport) {
		if (bTCPConnect == true)
			fprintf(stderr, "you cannot do this right now!\n");
		else
			Port = newport;
		return;
	}
	int GetPort() {return Port;}
	SocketType GetType() {return mySocket;}
	void SetType(SocketType newtype) {
		if (bTCPConnect == true)
			fprintf(stderr, "you cannot change this right now!\n");
		else 
			mySocket = newtype;
		return;
	}
};