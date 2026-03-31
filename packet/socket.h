//COIL milestone 2, socket definitions

#pragma once
#ifdef _WIN32
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <winsock2.h>
	#include <ws2tcpip.h> 
	//in your most recent example, you used "<ONWINDOWS.networking.sockets.h>", 
	//but I couldn't get sockaddrlen to work wiht that
	#pragma comment(lib, "Ws2_32.lib")
	#define ONWINDOWS 1 //for some reason, having it as 'true' causes issues in ifs
	//int //close(SOCKET s) { return 0; }
#elif __linux__ 
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>

	#include <arpa/inet.h> //needed for inet_addr()
	#define INVALID_SOCKET ~0 //this is how ONWINDOWS does it idk man
	#define SOCKET_ERROR -1
	#define SOCKET int
	int closesocket(int) { return 0; }
	bool ONWINDOWS = false;
#endif



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
	int MaxSize;

	SOCKET WelcomeSocket;
	SOCKET ConnectionSocket;
	bool bTCPConnect;

	std::string IPAddr;
	unsigned int Port;
	struct sockaddr_in SvrAddr;
	//udp needs the following
	struct sockaddr_in CliAddr;
	int AddrLen;

	SocketType mySocket;
	ConnectionType connectionType;


	int socketStart() {


		//socket
		if (mySocket == CLIENT) {

			if(connectionType==TCP)
				ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			else
				ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

			if (ConnectionSocket == INVALID_SOCKET) {
				return -1;
			}

			SvrAddr.sin_family = AF_INET;
			SvrAddr.sin_port = htons(Port);
			SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());

			//udp is ready to send/recieve, tcp still has to connect()
			return 0;
		}
		else if (connectionType == UDP && mySocket == SERVER) {

			ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (ConnectionSocket == INVALID_SOCKET) {
				return -1;
			}

			SvrAddr.sin_family = AF_INET;
			SvrAddr.sin_addr.s_addr = INADDR_ANY;
			SvrAddr.sin_port = htons(Port);

			//bind
			if (bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR){
				if (ONWINDOWS) {
					closesocket(ConnectionSocket);
				}
				else
					//close(ConnectionSocket);
				return -1;
			}

			//udp server is able to recvfrom at this stage
			return 0;
		}
		else if (connectionType == TCP && mySocket == SERVER) {
			WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (WelcomeSocket == INVALID_SOCKET) {
				return -1;
			}

			SvrAddr.sin_family = AF_INET;
			SvrAddr.sin_addr.s_addr = INADDR_ANY;
			SvrAddr.sin_port = htons(Port);

			//bind
			if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
				if (ONWINDOWS) 
					closesocket(WelcomeSocket);
				else
					//close(WelcomeSocket);
				return -1;
			}

			//listen
			if (listen(WelcomeSocket, 1) == SOCKET_ERROR) { //I think 1 is fine here?
				if (ONWINDOWS)
					closesocket(WelcomeSocket);
				else
					//close(WelcomeSocket);
				return -1;
			}

			//tcp server is able to accept() from this stage
			return 0;
		}
	}

public:
	MySocket(SocketType stype, std::string IP, unsigned int port, ConnectionType ctype, unsigned int size) {
		//basic values
		mySocket = stype;
		IPAddr = IP;
		Port = port;
		connectionType = ctype;

		//just initializing all of these because otherwise stuff might go badly
		WelcomeSocket;
		ConnectionSocket;
		SvrAddr;
		CliAddr;
		AddrLen = sizeof(SvrAddr);
		
		if (size > DEFAULT_SIZE) {
			Buffer = new char[size];
			MaxSize = size;
		}
		else {
			Buffer = new char[DEFAULT_SIZE];
			MaxSize = DEFAULT_SIZE;
		}

		int ok=socketStart();
		if (ok != 0) {
			exit(EXIT_FAILURE);//can't return on a constructor, could this be an exception instead?
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
			if (mySocket == CLIENT) {

				if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
					if (ONWINDOWS)
						closesocket(ConnectionSocket);
					else
						//close(ConnectionSocket);
					fprintf(stderr, "failed to connect\n");
					return;
				}
				bTCPConnect = true;
			//client can now send and recieve
			}
			else if (mySocket == SERVER) {
				ConnectionSocket = SOCKET_ERROR;
				if ((ConnectionSocket = accept(WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
					if (ONWINDOWS)
						closesocket(WelcomeSocket);
					else 
						//close(WelcomeSocket);
					fprintf(stderr, "failed to accept\n");
					return;
				}
				bTCPConnect = true;
			//server can now send and recieve
			}
			return;
		}
	}
	void DisconnectTCP() {
		if (connectionType == UDP || bTCPConnect == false) {
			fprintf(stderr, "you can't disconnect right now\n");
			return;
		}
		else{
			if(ONWINDOWS)
				closesocket(ConnectionSocket);
			else
				//close(ConnectionSocket);

			//I'm assuming this function doesn't want to 'end everything',
			//so I'm not closing welcomesocket or anything
			bTCPConnect == false;

			return;
		}
	}
	void KillTCPServ() {
		if (connectionType == TCP && bTCPConnect == false && mySocket == SERVER) {
			if (ONWINDOWS)
				closesocket(WelcomeSocket);
			//else
				//close(WelcomeSocket);
		}
		else {
			fprintf(stderr, "you cannot do that\n");
		}
		return;
	}

	void SendData(const char* dat, int size) {
		if (connectionType == TCP && bTCPConnect == true) {
			//dont want to call this without having connected
			send(ConnectionSocket, dat, size, 0);
		}
		else if (connectionType == UDP && mySocket==CLIENT) {
			int res=sendto(ConnectionSocket, dat, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
			printf("size send: %d\n", res);
		}
		else if (connectionType == UDP && mySocket == SERVER) {
			sendto(ConnectionSocket, dat, size, 0, (struct sockaddr*)&CliAddr, AddrLen);
		}
		else
			fprintf(stderr, "cannot do that right now\n");
		return;
	}
	int GetData(char* buf) {
		int recsize=0;
		if (connectionType == TCP && bTCPConnect == true) {
			recsize=recv(ConnectionSocket, Buffer, MaxSize, 0);
			//if the send data is bigger than maxsize, this might be a problem, but CRCs will tell us to throw away bad packets anyways
			printf("at tcp\n");
		}
		else if (connectionType == UDP && mySocket == CLIENT) {
			recsize = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &AddrLen);
			printf("recieved, size: %d\n", recsize);
		}
		else if (connectionType == UDP && mySocket == SERVER) {
			recsize = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&CliAddr, &AddrLen);
			printf("at udp serv\n");
		}

		if (recsize < 0)
			return -1;
		memcpy(buf, Buffer, recsize);
		return recsize;
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