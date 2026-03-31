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

        TEST_METHOD(CalcCRC)
        {
            char rawBuff[8] = {
                0x00, 0x00,
                0x00,
                0x08,
                0x01, 0x0A, 0x50,
                0x00 // set CRC to 0, to be recalculated
            };

            PktDef pkt(rawBuff);
            pkt.CalcCRC();
           
            char expected = 0x06;
            char actual = pkt.GetCRC();

            Assert::AreEqual(expected, actual);
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

    };
}