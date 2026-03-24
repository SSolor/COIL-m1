//sebastian, ricardo, rami, devki
//COIL milestone 1, packet definition

#include <stdio.h>
#include <memory>
#include <bitset>
//#include <cstring> 
//I remember another compiler needed this for memset even tho it should be in <memory>


// #defines > const ints fite me
#define FORWARD 1
#define	BACKWARD 2
#define RIGHT 3
#define LEFT 4
//couldn't these just have been enums though? well whatever
#define HEADERSIZE 4 // may change if padding and stuff gets messed up

//not sure if we need these? keeping them just in case
#define DRIVE_MIN 80
#define DRIVE_MAX 100

struct DriveBody {
	//going with unsigned char since I know its 1 byte
	unsigned char Direction;
	unsigned char Duration;
	unsigned char Power;
};

struct TurnBody {
	unsigned char Direction;
	unsigned short int Duration; //hopefully this is doesn't get padded
};

typedef enum CmdType {
	//unsure if these should just be numbers or represent the bitfield
	DRIVE,
	RESPONSE, //for "status"
	SLEEP
};

class PktDef {
	struct Header {
		unsigned short PktCount; //problem: int is 4byte, req is 2. solution: short
		//may need to change depending on OS
		unsigned char Drive : 1;
		unsigned char Status : 1;
		unsigned char Sleep : 1;
		unsigned char Ack : 1; //was gettin some compiler warnings so I unsigned all of these
		unsigned char Padding : 4;
		unsigned char Length;//problem: short is 2byte, req is 1. solution: char
		//may need to change depending on OS.
		//is this considered not following the requirements? I hope not
		//definition of 'short int' is literally >=2 bytes, so its impossible to begin with
		//also, just going along with the requirements that its the size of the entire packet, 
		// although that makes things a bit inconvenient
	}Head;

	char* Data; 
	char CRC;

	char* RawBuffer;
public:
	PktDef() {
		memset(&Head, 0, HEADERSIZE);
		Data = nullptr;
		RawBuffer = nullptr;
		CRC = 0;
	}

	PktDef(char* Rawdat) {
		memset(&Head, 0, HEADERSIZE); //just in case
		RawBuffer = nullptr;
		Data = nullptr;
		CRC = 0;

		if (Rawdat == nullptr) return;

		//head
		memcpy(&Head, Rawdat, HEADERSIZE);

		if (Head.Length == 0) return;

		//body
		memcpy(Data, (Rawdat + HEADERSIZE), (Head.Length - HEADERSIZE - sizeof(CRC)));
		//a bit awkward since the size Head gives us is total size, but whatever

	//tail
		memcpy(&CRC, (Rawdat + Head.Length - sizeof(CRC)), sizeof(CRC));


	}

	void SetCmd(CmdType cmd) {
		Head.Drive = 0;
		Head.Status = 0;
		Head.Sleep = 0;

		switch (cmd) {
		case DRIVE:
			Head.Drive = 1;
			break;
		case RESPONSE:
			Head.Status = 1;
			break;
		case SLEEP:
			Head.Sleep = 1;
			break;
		}
	//ensures only 1 flag set at a time
	}

	void SetBodyData(char* dat, int size) {
		if (Data) {
			delete[] Data;
			Data = nullptr;
		}

		if (!dat || size <= 0) {
			Head.Length = HEADERSIZE + sizeof(CRC);
			return;
		}

		Data = new char[size];
		memcpy(Data, dat, size);

		Head.Length += size;//can't just let it = since its *packet* size
	}

	void SetPktCount(int count) {
		Head.PktCount = count;
	}

	CmdType GetCmd() {
		if (Head.Drive == 1) //0x1 is 1 in Hex
			return DRIVE;
		else if (Head.Status == 1)
			return RESPONSE;
		else if (Head.Sleep == 1)
			return SLEEP;
	}

	bool GetAck() {
		return Head.Ack;//0 evaluates to false. unless cpp gets annoying about it? works in c anyways
	}

	int GetLength() {
		return Head.Length;
	}

	char* GetBodyData() {
		return Data;
	}

	int GetPktCount() {
		return Head.PktCount;
	}

	// *** Changed ***
	bool CheckCRC(char* dat, int size) {
		if (!dat || size <= 0) return false;

		unsigned char count = 0;

		for (int i = 0; i < size - (int)sizeof(CRC); i++) {
			unsigned char byte = (unsigned char)dat[i];

			for (int j = 0; j < 8; j++) {
				if (byte & (1 << j)) {
					count++;
				}
			}
		}

		return count == (unsigned char)dat[size - sizeof(CRC)];
	}

	// *** Changed ***
	void CalcCRC() {
		unsigned char count = 0;

		char* hdrPtr = (char*)&Head;
		for (int i = 0; i < HEADERSIZE; i++) {
			unsigned char byte = (unsigned char)hdrPtr[i];

			for (int j = 0; j < 8; j++) {
				if (byte & (1 << j)) {
					count++;
				}
			}
		}

		int bodyLength = Head.Length - HEADERSIZE - sizeof(CRC);
		if (Data && bodyLength > 0) {
			for (int i = 0; i < bodyLength; i++) {
				unsigned char byte = (unsigned char)Data[i];

				for (int j = 0; j < 8; j++) {
					if (byte & (1 << j)) {
						count++;
					}
				}
			}
		}

		CRC = count;
	}

	// *** Changed ***
	char* GenPacket() {
		if (RawBuffer) {
			delete[] RawBuffer;
			RawBuffer = nullptr;
		}

		int totalLength = Head.Length;
		if (totalLength < HEADERSIZE + (int)sizeof(CRC)) return nullptr;

		RawBuffer = new char[totalLength];

		memcpy(RawBuffer, &Head, HEADERSIZE);

		int bodyLength = totalLength - HEADERSIZE - sizeof(CRC);
		if (Data && bodyLength > 0) {
			memcpy(RawBuffer + HEADERSIZE, Data, bodyLength);
		}

		CalcCRC();
		RawBuffer[totalLength - sizeof(CRC)] = CRC;

		return RawBuffer;
	}

	~PktDef() {
		if (RawBuffer) delete[] RawBuffer;
		if (Data) delete[] Data;
	}
};
