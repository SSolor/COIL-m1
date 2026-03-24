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
		short int PktCount; //problem: int is 4byte, req is 2. solution: short
		//may need to change depending on OS
		unsigned int Drive : 1;
		unsigned int Status : 1;
		unsigned int Sleep : 1;
		unsigned int Ack : 1; //was gettin some compiler warnings so I unsigned all of these
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
		//head
		memcpy(&Head, Rawdat,HEADERSIZE);
		//body
		memcpy(Data, (Rawdat + HEADERSIZE), (Head.Length - HEADERSIZE - sizeof(CRC)));
			//a bit awkward since the size Head gives us is total size, but whatever
		//tail
		memcpy(&CRC, (Rawdat + Head.Length - sizeof(CRC)), sizeof(CRC));

		RawBuffer = nullptr;//just in case
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
		}

		if (!dat || size <= 0) {
			Data = nullptr;
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
		if (Head.Drive == 0x1)
			return DRIVE;
		else if (Head.Status == 0x1)
			return RESPONSE;
		else if (Head.Sleep == 0x1)
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
		int count=0;
		for (int i = 0;i < size;i++) {
			if (dat[i] == 0x1)
				count++;
		}//this feels really dumb but I think it works?

		if (count == CRC)
			return true;
		else
			return false;
	}

	void CalcCRC() {
		int size = Head.Length - sizeof(CRC);

		int count = 0;
		for (int i = 0;i < size;i++) {
			if (RawBuffer[i] == 0x1)//honestly not sure this works but I think it should
				count++;
		}
		CRC = count;
	}

	char* GenPacket() {
		if (RawBuffer) {
			delete[] RawBuffer;
		}

		Head.Length += HEADERSIZE + sizeof(CRC);
		//in order to create and send a packet, the steps are: packet(), setbodydata(char*, int), this
		//packet keeps size at 0, setbodydata adds the size of the body. so we're still missing size of head and crc

		RawBuffer = new char[Head.Length];

		memcpy(RawBuffer, &Head, HEADERSIZE);
		memcpy(RawBuffer + HEADERSIZE, Data, (Head.Length - HEADERSIZE - sizeof(CRC)));

		CalcCRC();
		memcpy((RawBuffer + Head.Length - sizeof(CRC)), &CRC, sizeof(CRC));

		return RawBuffer;
	}
};
