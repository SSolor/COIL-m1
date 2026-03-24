#include "pch.h"
#include "CppUnitTest.h"
#include "../packet/packet.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
    TEST_CLASS(PktDefTests)
    {
    public:
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

        TEST_METHOD(SetPktCount)
        {
            PktDef pkt;
            pkt.SetPktCount(25);

            int expected = 25;
            int actual = pkt.GetPktCount();

            Assert::AreEqual(expected, actual);
        }

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
        TEST_METHOD(SetBodyData_Empty)
        {
            PktDef pkt;

            pkt.SetBodyData(nullptr, 0);

            Assert::IsNull(pkt.GetBodyData());
        }

    };
}
