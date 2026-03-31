#include "pch.h"
#include "CppUnitTest.h"
#include "../packet/packet.cpp"

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

            int expected = 0;
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
<<<<<<< Updated upstream
=======

        TEST_METHOD(DefaultConstructor_TooSmall)
        {
            char raw[4] = {0}; //smaller than HEADERSIZE+sizeof(char)

            pktDef pkt(raw);
        }

>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
=======

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
            pktdef pkt;

            // This test is to ensure that the function can handle an invalid enum value without crashing.
            CmdType cmd = pkt.GetCmd();
        }

        TEST_METHOD(SetCmd_Invalid)
        {
            pktdef pkt;

            pkt.SetCmd((CmdType)999); // Invalid enum value
        }
        
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
        TEST_METHOD(SetBodyData_Empty)
=======
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
            pktDef pkt;
            pkt.SetBodyData(nullptr, 0);

            Assert::IsTrue(pkt.GetLength() >= HEADERSIZE + sizeof(char));
        }

        TEST_METHOD(SetBodyData_Nullptr)
        {
            pktDef pkt;

            // This test is to ensure that the function can handle a nullptr without crashing.
            pkt.SetBodyData(nullptr, 3);
        }

        TEST_METHOD(CalcCRC)
>>>>>>> Stashed changes
        {
            PktDef pkt;

            pkt.SetBodyData(nullptr, 0);

            Assert::IsNull(pkt.GetBodyData());
        }
        
        // CalcCRC tests.
        TEST_METHOD(CalcCRC_Default)
        {
            PktDef pkt;

            pkt.CalcCRC();

            char expected = 0;
            char actual = pkt.CRC;

            Assert::AreEqual(expected, actual);
        }

<<<<<<< Updated upstream
        TEST_METHOD(CalcCRC_WithData)
=======
        //CheckCRC tests
        TEST_METHOD(CheckCRC)
>>>>>>> Stashed changes
        {
            PktDef pkt;
            pkt.body[3] = { 1, 10, 80 };

            pkt.CalcCRC();

            char expected = 5;
            char actual = pkt.CRC;

            Assert::AreEqual(expected, actual);
        }

        // CheckCRC tests.
        TEST_METHOD(CheckCRC_Valid)
        {
            PktDef pkt;
            pkt.body[3] = { 1, 10, 80 };
            pkt.CalcCRC(); // for debugging (breaker)

            bool result = pkt.CheckCRC(pkt.GetBodyData(), 3);

            Assert::IsTrue(result);
        }

        TEST_METHOD(CheckCRC_Default)
        {
            PktDef pkt;
            pkt.CalcCRC(); // for debugging (breaker)

            bool result = pkt.CheckCRC(pkt.GetBodyData(), 3);

            Assert::IsTrue(result);
        }
<<<<<<< Updated upstream
=======

        TEST_METHOD(CheckCRC_ModifiedBuffer)
        {
            PktDef pkt;
            char body[3] = { 1, 10, 80 };
            
            pkt.SetBodyData(body, 3);
            char* raw = pkt.GenPacket();

            raw[0] = 0xFF; //flip bits

            bool valid = pkt.checkCRC(raw, pkt.GetLength());

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

        TEST_METHOD(GenPacket_NoBody)
        {
            PktDef pkt;
            pkt.SetBodyData(body, 3);

            char* result = pkt.GenPacket();

            Assert::IsNotNULL(result);
        }

>>>>>>> Stashed changes
    };
}
