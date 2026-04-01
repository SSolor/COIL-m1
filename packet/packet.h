//sebastian, ricardo, rami, devki
//COIL milestone 1, packet definition
#pragma once
#include <memory>
#include <cstring> 

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

#pragma pack(push, 1) //force size to 3
	struct TurnBody {
		uint8_t Direction;
		uint16_t Duration;
	};
#pragma pack(pop)


typedef enum CmdType {
	//unsure if these should just be numbers or represent the bitfield
	DRIVE,
	SLEEP,
	RESPONSE //for "status"
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
		CRC = 0;

		Head.Length = HEADERSIZE + sizeof(CRC);
		//since it represents entire length we can't forget this

		RawBuffer = nullptr;
	}

	PktDef(char* Rawdat) {
		//head
		memcpy(&Head, Rawdat, HEADERSIZE);

		//body
		Data = new char[Head.Length - HEADERSIZE - sizeof(CRC)];
		memcpy(Data, Rawdat + HEADERSIZE, Head.Length - HEADERSIZE - sizeof(CRC));

		//tail
		memcpy(&CRC, (Rawdat + Head.Length - sizeof(CRC)), sizeof(CRC));

		RawBuffer = nullptr;//keep this empty
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
			Head.Length -= (Head.Length - HEADERSIZE - sizeof(CRC));//aka body size
			delete[] Data;
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


	bool CheckCRC(char* dat, int size) {
		int counter = 0;

		for (int byte = 0; byte < size - sizeof(CRC); byte++) {
			for (int bit = 0; bit < 8; bit++) {
				if (dat[byte] & (1 << bit)) {
					counter++;
				}
			}
		}

		return (char)counter == CRC;
	}

	void CalcCRC() {
		int counter = 0;

		//it is presumed that this is *only* supposed to be called as the last phase of genpkt
		//wherein it wants to certify the integrity of RawBuffer (thus, it checks RawBuffer)
		for (int byte = 0; byte < (Head.Length - sizeof(CRC));byte++) {
			for (int bit = 0; bit < 8;bit++) {
				if (RawBuffer[byte] & (1 << bit)) {
					counter++;
				}
			}
		}

		//if this is not the case, and it just checks the packet, generally, use this instead:
		/*
		char* Hdr = (char*)&Head;

		for (int byte = 0; byte < HEADERSIZE; byte++) {
			for (int bit = 0; bit < 8; bit++) {
				if (Hdr[byte] & (1 << bit)) {
					counter++;
				}
			}
		}

		int Len = Head.Length - HEADERSIZE - sizeof(CRC);

		for (int byte = 0; byte < Len; byte++) {
			for (int bit = 0; bit < 8; bit++) {
				if (Data[byte] & (1 << bit)) {
					counter++;
				}
			}
		}*/

		CRC = counter;
	}

	char* GenPacket() {
		if (RawBuffer) {
			delete[] RawBuffer;
		}

		//header+crc accounted for in constructor, data accounted for in setbodydata
		RawBuffer = new char[Head.Length];

		//putting head into buffer
		memcpy(RawBuffer, &Head, HEADERSIZE);
		//putting body into buffer
		memcpy(RawBuffer + HEADERSIZE, Data, (Head.Length - HEADERSIZE - sizeof(CRC)));

		//calculating crc,
		CalcCRC();
		//putting it into the buffer
		memcpy(RawBuffer + Head.Length - sizeof(CRC), &CRC, sizeof(CRC));

		
		return RawBuffer;
	}

	char GetCRC() {
		return CRC;
	}

	~PktDef() {
		if (RawBuffer) delete[] RawBuffer;
		if (Data) delete[] Data;
	}
};
