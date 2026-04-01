#include "pch.h"
#include "CppUnitTest.h"
#include "../packet/packet.h"
#include "../packet/socket.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{

    TEST_CLASS(PktDefTests)
    {
    public:
        // Default constructor tests.
        TEST_METHOD(DefaultConstructor_PktCount)
        {
            PktDef pkt;

            int expected = 0;
            int actual = pkt.GetPktCount();

            Assert::AreEqual(expected, actual);
        }

        TEST_METHOD(DefaultConstructor_Length)
        {
            PktDef pkt;

            int expected = HEADERSIZE+sizeof(char);
            int actual = pkt.GetLength();

            Assert::AreEqual(expected, actual);
        }

        TEST_METHOD(DefaultConstructor_Ack)
        {
            PktDef pkt;

            Assert::IsFalse(pkt.GetAck());
        }

        TEST_METHOD(DefaultConstructor_Body)
        {
            PktDef pkt;
            Assert::IsNull(pkt.GetBodyData());
        }

   
        // SetCmd tests.
        TEST_METHOD(SetCmd_DRIVE)
        {
            PktDef pkt;

            pkt.SetCmd(DRIVE);
            int expected = DRIVE;
            int actual = pkt.GetCmd();

            Assert::AreEqual(expected, actual);
        }

        TEST_METHOD(SetCmd_SLEEP)
        {
            PktDef pkt;

            pkt.SetCmd(SLEEP);
            int expected = SLEEP;
            int actual = pkt.GetCmd();

            Assert::AreEqual(expected, actual);
        }

        TEST_METHOD(SetCmd_RESPONSE)
        {
            PktDef pkt;
            pkt.SetCmd(RESPONSE);
            int expected = RESPONSE;
            int actual = pkt.GetCmd();
            Assert::AreEqual(expected, actual);
        }

        TEST_METHOD(SetCmd_Switch)
        {
            PktDef pkt;
            pkt.SetCmd(DRIVE);
            pkt.SetCmd(RESPONSE);
            int expected = RESPONSE;
            int actual = pkt.GetCmd();
            Assert::AreEqual(expected, actual);
        }

        TEST_METHOD(SetCmd_Default)
        {
            PktDef pkt;

            // This test is to ensure that the function can handle an invalid enum value without crashing.
            CmdType cmd = pkt.GetCmd();
        }

        TEST_METHOD(SetCmd_Invalid)
        {
            PktDef pkt;

            pkt.SetCmd((CmdType)999); // Invalid enum value
        }

        // SetPktCount test.
        TEST_METHOD(SetPktCount)
        {
            PktDef pkt;
            pkt.SetPktCount(25);

            int expected = 25;
            int actual = pkt.GetPktCount();

            Assert::AreEqual(expected, actual);
        }
        // SetBodyData tests.
        TEST_METHOD(SetBodyData_NotNull)
        {
            PktDef pkt;
            char body[3] = { 1, 10, 80 };

            pkt.SetBodyData(body, 3);

            char* result = pkt.GetBodyData();
            Assert::IsNotNull(result);
        }

        TEST_METHOD(SetBodyData_CorrectData)
        {
            PktDef pkt;
            char body[3] = { 1, 10, 80 };

            pkt.SetBodyData(body, 3);
            char* result = pkt.GetBodyData();

            Assert::AreEqual((char)1, result[0]);
            Assert::AreEqual((char)10, result[1]);
            Assert::AreEqual((char)80, result[2]);
        }

        TEST_METHOD(SetBodyData_MultipleCalls)
        {
            PktDef pkt;
            char body[3] = { 1, 10, 80 };

            pkt.SetBodyData(body, 3);
            int firstLength = pkt.GetLength();

            pkt.SetBodyData(body, 3);
            int secondLength = pkt.GetLength();

            Assert::AreEqual(firstLength, secondLength);
        }

        TEST_METHOD(SetBodyData_Size0)
        {
            PktDef pkt;
            pkt.SetBodyData(nullptr, 0);

            Assert::IsTrue(pkt.GetLength() >= HEADERSIZE + sizeof(char));
        }
    
        TEST_METHOD(CheckCRC)
        {
         const int size = 8;
            char rawBuff[size] = {
                0x00, 0x00,       
                0x00,             
                0x08,             
                0x01, 0x0A, 0x50, 
                0x06              
            };

            PktDef pkt(rawBuff);

            bool result = pkt.CheckCRC(rawBuff, size);

            Assert::IsTrue(result);
        }

        TEST_METHOD(CheckCRC_Invalid)
        {
            const int size = 8;
            char rawBuff[size] = {
                0x00, 0x00,
                0x00,
                0x08,
                0x01, 0x0A, 0x50,
                0x05 // CRC off by 1
            };

            PktDef pkt(rawBuff);

            bool result = pkt.CheckCRC(rawBuff, size);

            Assert::IsFalse(result);
        }

        TEST_METHOD(GenPacket)
        {
            const int size = 8;
            char expected[size] = {
                0x00, 0x00,
                0x00,
                0x08,
                0x01, 0x0A, 0x50,
                0x06 
            };

            PktDef pkt(expected);

            char* actual = pkt.GenPacket();

            for (int byte = 0; byte < size; byte++) {
                Assert::AreEqual(expected[byte], actual[byte]);
            }
        }

        TEST_METHOD(CheckCRC_ModifiedBuffer)
        {
            PktDef pkt;
            char body[3] = { 1, 10, 80 };

            pkt.SetBodyData(body, 3);
            char* raw = pkt.GenPacket();

            raw[0] = 0xFF; //flip bits

            bool valid = pkt.CheckCRC(raw, pkt.GetLength());

            Assert::IsFalse(valid);
        }

        //GenPacket tests
        TEST_METHOD(GenPacket_Valid)
        {
            const int size = 8;
            char expected[size] = {
                0x00, 0x00,
                0x00,
                0x08,
                0x01, 0x0A, 0x50,
                0x06
            };

            PktDef pkt(expected);

            char* actual = pkt.GenPacket();

            for (int byte = 0; byte < size; byte++) {
                Assert::AreEqual(expected[byte], actual[byte]);
            }
        }

    };
    TEST_CLASS(socketTests) {
    public:
        TEST_METHOD(constructorValues_areValid) {
            SocketType expectedSType=CLIENT;
            std::string expectedIP = "10.172.41.150";
            unsigned int expectedPort = 29000;
            ConnectionType expectedCType = TCP;
            unsigned int expectedSize = 100;

            MySocket soc(expectedSType,expectedIP,expectedPort,expectedCType,expectedSize);

            Assert::AreEqual((int)expectedSType, (int)soc.GetType());
            Assert::AreEqual(expectedIP, soc.GetIPAddr());
            Assert::AreEqual(expectedPort, soc.GetPort());
            Assert::AreEqual((int)expectedCType, (int)soc.GetCType());
            Assert::AreEqual(expectedSize, soc.getMaxSize());
        }
        TEST_METHOD(setType_isValid) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            SocketType expected = SERVER;
            soc.SetType(expected);

            Assert::AreEqual((int)expected, (int)soc.GetType());
        }
        TEST_METHOD(setIP_isValid) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            std::string expected = "127.0.0.1";
            soc.SetIPAddr(expected);

            Assert::AreEqual(expected, soc.GetIPAddr());
        }
        TEST_METHOD(setPort_isValid) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            unsigned int expected = 30000;
            soc.SetPort(expected);

            Assert::AreEqual(expected, soc.GetPort());
        }
        TEST_METHOD(setCType_isValid) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            ConnectionType expected = UDP;
            soc.SetCType(expected);

            Assert::AreEqual((int)expected, (int)soc.GetCType());
        }
        TEST_METHOD(defaultSize_isValid) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 1);
            unsigned int expected = 5;

            Assert::AreEqual(expected, soc.getMaxSize());
        }

        //i'm honestly not sure how the following are working without wsa. does the unit tester take care of that?
        //I tried copying the code into a regular main and it doesn't work, so the unit tester must be doing something?
        //its the exact same code you can see for yourself
        //if these tests don't work for you, then add WSA start I guess?

        //anyways since connecttcp is difficult to test on its own (since its void) we use this to check it worked
        TEST_METHOD(connectionBlocks_SetType) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();

            SocketType Nexpected = SERVER;
            SocketType expected = CLIENT;
            soc.SetType(Nexpected);

            Assert::AreEqual((int)expected, (int)soc.GetType());
        }
        TEST_METHOD(connectionBlocks_SetIP) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();

            std::string Nexpected = "127.0.0.1";
            std::string expected = "10.172.41.150";
            soc.SetIPAddr(Nexpected);

            Assert::AreEqual(expected, soc.GetIPAddr());
        }
        TEST_METHOD(connectionBlocks_SetPort) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();

            unsigned int Nexpected = 30000;
            unsigned int expected =  29000;
            soc.SetPort(Nexpected);

            Assert::AreEqual(expected, soc.GetPort());
        }
        TEST_METHOD(connectionBlocks_SetCType) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();

            ConnectionType Nexpected = UDP;
            ConnectionType expected = TCP;
            soc.SetCType(Nexpected);

            Assert::AreEqual((int)expected, (int)soc.GetCType());
        }
        TEST_METHOD(FailedConnection_NotBLockSet) {
            MySocket soc(CLIENT, "2", 2, TCP, 100);
            soc.ConnectTCP();

            ConnectionType Nexpected = TCP;
            ConnectionType expected = UDP;
            soc.SetCType(expected);

            Assert::AreEqual((int)expected, (int)soc.GetCType());
        }
        //these are essentially the same so I'll only test 1
        TEST_METHOD(welcomeBlocks_SetType) {
            MySocket soc(SERVER, "10.172.41.150", 29000, TCP, 100);

            SocketType Nexpected = CLIENT;
            SocketType expected = SERVER;
            soc.SetType(Nexpected);

            Assert::AreEqual((int)expected, (int)soc.GetType());
        }
        TEST_METHOD(setTypeBlocks_sockets) {
            MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);

            SocketType change = SERVER;
            soc.SetType(change);

            char buf[100];
            int result = soc.GetData(buf);//using getdata because its the only one of these that isn't void
            int expected = -2;

            Assert::AreEqual(expected, result);
        }
        TEST_METHOD(setCTypeBlocks_sockets) {
            MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);

            ConnectionType change = TCP;
            soc.SetCType(change);

            char buf[100];
            int result = soc.GetData(buf);//using getdata because its the only one of these that isn't void
            int expected = -2;

            Assert::AreEqual(expected, result);
        }
        TEST_METHOD(sameSetNotBlocks_sockets) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);

            SocketType change = CLIENT;
            soc.SetType(change);

            char buf[100];
            int result = soc.GetData(buf);//using getdata because its the only one of these that isn't void
            //now, since its tcp but we didn't connect, we expect an error still
            int expected = -1;

            Assert::AreEqual(expected, result);
        }
        TEST_METHOD(resetUnBlocks_sockets) {
            MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);

            ConnectionType change = TCP;
            soc.SetCType(change);
            soc.resetSocket();

            char buf[100];
            int result = soc.GetData(buf);//using getdata because its the only one of these that isn't void
            //now, since its tcp but we didn't connect, we expect an error still
            int expected = -1;

            Assert::AreEqual(expected, result);
        }
        //again,these should all be the same so i'll only test one
        TEST_METHOD(disconnectUnBlocks_Setters) {
            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();
            soc.DisconnectTCP();

            SocketType Nexpected = CLIENT;
            SocketType expected = SERVER;
            soc.SetType(expected);

            Assert::AreEqual((int)expected, (int)soc.GetType());
        }
        //tests against the sim
        TEST_METHOD(TCPSendRecv_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(UDPSendRecv_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(TCPDisconnect_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();
            soc.DisconnectTCP();

            soc.SendData(x, p.GetLength()); //should fail

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected =-1; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(TCPDisconnectReconnect_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();
            soc.DisconnectTCP();
            soc.ConnectTCP();
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(UDPConnect_DoesNothing) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);
            soc.ConnectTCP();
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(UDPDisconnect_DoesNothing) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);
            soc.DisconnectTCP();
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(UDPReset_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29500, UDP, 100);
            soc.resetSocket();//kills and remakes, which is all UDP needs
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(TCPRawReset_DoesNothing) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();
            soc.resetSocket();//not allowed on tcp, connection stays alive
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(TCProperReset_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 29000, TCP, 100);
            soc.ConnectTCP();
            soc.DisconnectTCP();
            soc.resetSocket();//kills and remakes
            soc.ConnectTCP();
            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(ChangeType_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(SERVER, "10.172.41.150", 29500, UDP, 100);
            soc.SetType(CLIENT);
            soc.resetSocket();

            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }
        TEST_METHOD(ChangePort_isValid) {
            //we can use packet just fine here because its already been tested
            PktDef p;
            p.SetCmd(RESPONSE);
            p.SetPktCount(5);

            char* x = p.GenPacket();

            MySocket soc(CLIENT, "10.172.41.150", 12500, UDP, 100);
            soc.SetPort(29500);
            soc.resetSocket();

            soc.SendData(x, p.GetLength());

            char buf[150];//extra because why not
            int recieved = soc.GetData(buf);

            int expected = 61; //not gonna check the whole thing, just its size (anything at all means it worked, really)
            Assert::AreEqual(expected, recieved);
        }

    };
}