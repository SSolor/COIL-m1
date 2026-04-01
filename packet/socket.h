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
	//int //close(SOCKET s) { return 0; } //this causes issues for some reason

#elif __linux__ 
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>

	#include <arpa/inet.h> //needed for inet_addr()
	#include <cstring>
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
	TCP=2,
	UDP
};
const int DEFAULT_SIZE = 5;//i'm not sure exactly, but is should at least hold an empty packet, right?

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
	int AddrLen;//I think it's fine if server and client share this

	SocketType mySocket;
	ConnectionType connectionType;

	bool welcomeopen;
	bool changesmade;


	//initializes sockets based on stored values
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
				else {
					//close(ConnectionSocket);
				}
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
				else {
					//close(WelcomeSocket);
				}
				return -1;
			}

			//listen
			if (listen(WelcomeSocket, 1) == SOCKET_ERROR) { //I think 1 is fine here?
				if (ONWINDOWS)
					closesocket(WelcomeSocket);
				else {
					//close(WelcomeSocket);
				}
				return -1;
			}
			welcomeopen = true;
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
		bTCPConnect = false;
		
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
			fprintf(stderr, "socket or bind failed\n");
			return;
		}
	}
	~MySocket() {
		if (Buffer)
			delete[] Buffer;
		//I don't believe anything else is dynamic
	}

	void ConnectTCP() {
		if (connectionType == UDP || bTCPConnect == true || changesmade==true) {
			//I think for all these writing to stderr is acceptable? unless there's a cpp way of doing it
			fprintf(stderr, "UDP or already connected or changes made\n");
			return;
		}
		else {
			if (mySocket == CLIENT) {
				if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
					if (ONWINDOWS)
						closesocket(ConnectionSocket);
					else {
						//close(ConnectionSocket);
					}
					fprintf(stderr, "tcp failed to connect\n");
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
					else {
						//close(WelcomeSocket);
					}
					fprintf(stderr, "tcp failed to accept\n");
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
			else {
				//close(ConnectionSocket);
			}
			//I'm assuming this function doesn't want to 'end everything',
			//so I'm not closing welcomesocket or anything
			bTCPConnect = false;

			int okay=socketStart();//gets killed socket ready for connection again
			if (okay != 0)
				fprintf(stderr, "new socket failed\n");
			//if you want to change things about your socket, after doing this, set<whatever>(), resetsocket();
			//having it like this prevents double closing or anything

			return;
		}
	}
	void resetSocket() {
		if (bTCPConnect == true) {
			fprintf(stderr, "disconnect before reset\n");
			return;
		}
		if (ONWINDOWS)
			closesocket(ConnectionSocket);
		else {
			//close(ConnectionSocket);
		}
		int okay=socketStart();
		if (okay != 0)
			fprintf(stderr, "new socket failed\n");
		changesmade = false;
	}
	void KillTCPServ() {
		if (connectionType == TCP && bTCPConnect == false && mySocket == SERVER &&welcomeopen==true) {
			if (ONWINDOWS)
				closesocket(WelcomeSocket);
			else {
				//close(WelcomeSocket);
			}
			welcomeopen = false;
		}
		else {
			fprintf(stderr, "you cannot kill server now\n");
		}
		return;
	}

	void SendData(const char* dat, int size) {
		if (changesmade == true) {
			fprintf(stderr, "cannot send data, changes have been made\n");
			return;
		}

		if (connectionType == TCP && bTCPConnect == true) {
			//dont want to call this without having connected
			send(ConnectionSocket, dat, size, 0);
		}
		else if (connectionType == UDP && mySocket==CLIENT) {
			sendto(ConnectionSocket, dat, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
		}
		else if (connectionType == UDP && mySocket == SERVER) {
			sendto(ConnectionSocket, dat, size, 0, (struct sockaddr*)&CliAddr, AddrLen);
		}
		else
			fprintf(stderr, "cannot send data now\n");
		return;
	}
	int GetData(char* buf) {
		if (changesmade == true) {
			fprintf(stderr, "cannot get data, changes have been made\n");
			return -2; //to differentiate between recv error
		}

		int recsize=0;
		if (connectionType == TCP && bTCPConnect == true) {
			recsize=recv(ConnectionSocket, Buffer, MaxSize, 0);
			//if the send data is bigger than maxsize, this might be a problem, but CRCs will tell us to throw away bad packets anyways
		}
		else if (connectionType == UDP && mySocket == CLIENT) {
			recsize = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &AddrLen);
		}
		else if (connectionType == UDP && mySocket == SERVER) {
			recsize = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&CliAddr, &AddrLen);
		}
		//happens when recv/from is invalid
		if (recsize <= 0)
			return -1;

		memcpy(buf, Buffer, recsize);
		return recsize;
	}

	std::string GetIPAddr() {return IPAddr;}
	//will not be reflected until reset
	void SetIPAddr(std::string newaddr) {
		if (bTCPConnect == true || welcomeopen==true)
			fprintf(stderr, "you cannot change ip right now!\n");
		else
			IPAddr = newaddr;
		return;
	}
	//will not be reflected until reset
	void SetPort(unsigned int newport) {
		if (bTCPConnect == true ||welcomeopen==true)
			fprintf(stderr, "you cannot do port right now!\n");
		else
			Port = newport;
		return;
	}
	unsigned int GetPort() {return Port;}//technically changes return from isntructions but it should be fine?

	SocketType GetType() {return mySocket;}
	//will block certain functionality until reset
	void SetType(SocketType newtype) {
		if (bTCPConnect == true ||welcomeopen == true)
			fprintf(stderr, "you cannot change socketype right now!\n");
		else if (mySocket == newtype)
			return;
		else {
			mySocket = newtype;
			changesmade = true;
		}
		return;
	}

	ConnectionType GetCType() { return connectionType; }
	//will block certain functionality until reset
	void SetCType(ConnectionType newtype) {
		if (bTCPConnect == true || welcomeopen == true)
			fprintf(stderr, "you cannot change connectiontype right now!\n");
		else if (connectionType == newtype)
			return;
		else {
			connectionType = newtype;
			changesmade = true;
		}
		return;
	}
	unsigned int getMaxSize() { return MaxSize; }
};