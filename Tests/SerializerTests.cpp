#include "stdafx.h"
#include "CppUnitTest.h"
#include <OpenBattleCore.h>
#include <Generated.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace OpenBattle;

namespace Tests
{
	TEST_CLASS(SerializerTests)
	{
		class TestStream : public OpenBattle::Stream {
			uint8_t pos = 0;
		public:
			uint8_t *Data;
			uint8_t Len = 0;
			TestStream() {
				this->Data = new uint8_t[64];
				this->Len;
			}
			~TestStream() {
				delete[] this->Data;
			}
			void writeByte(unsigned char c) {
				this->Data[this->Len++] = c;
			}
			void write(unsigned char *buf, unsigned char len) {
				for (int i = 0; i < len; i++) {
					this->Data[this->Len + i] = buf[i];
				}
				this->Len += len;
			}
		};

	public:
		TEST_METHOD(SerializerBufferTest)
		{
			Serializer serializer;
			unsigned char s;
			Assert::IsFalse(serializer.read(&s), L"Empty serializer should not be able to read");

			serializer.append(new unsigned char[] { 0x01, 0x02 }, 2);
			Assert::IsTrue(serializer.read(&s), L"Serializer should have been able to read");
			Assert::IsTrue(s == 0x01, L"Serializer returned an incorrect value");

			Assert::IsTrue(serializer.read(&s), L"Serializer should have been able to read");
			Assert::IsTrue(s == 0x02, L"Serializer returned an incorrect value");

			Assert::IsFalse(serializer.read(&s), L"Empty serializer should not be able to read");
		}

		TEST_METHOD(SerializerUShortReadTest)
		{
			Serializer serializer;
			unsigned short s = 0;
			Assert::IsFalse(serializer.read(&s), L"Empty serializer should not be able to read");

			serializer.append(new unsigned char[] { 0xF4, 0x01 }, 2);
			Assert::IsTrue(serializer.read(&s), L"Serializer should have been able to read");
			Assert::IsTrue(s == 500, L"Value not deserialized correctly");
		}

		TEST_METHOD(SerializerUShortWriteTest)
		{
			Serializer serializer;
			TestStream stream;
			unsigned short i = 500;
			serializer.write(i, &stream);

			Assert::AreEqual((unsigned char)2, stream.Len, L"Stream has an incorrect length");
			Assert::IsTrue(stream.Data[1] == 0x01 && stream.Data[0] == 0xf4, L"UShort serialized incorrectly");
		}

		TEST_METHOD(SerializerStringReadTest)
		{
			Serializer serializer;
			char *s = 0;
			Assert::IsFalse(serializer.read(&s), L"Empty serializer should not be able to read");

			serializer.append(new unsigned char[] { 0x0c }, 1);
			serializer.append("Hello World!", 12);
			Assert::IsTrue(serializer.read(&s), L"Serailizer should have been able to read");
			Assert::AreEqual("Hello World!", (char*)s, L"Value not deserialized correctly");
		}

		TEST_METHOD(SerializerStringWriteTest) {
			Serializer serializer;
			TestStream stream;
			serializer.write("Hello World!", &stream);

			Assert::AreEqual((unsigned char)13, stream.Len, L"Invalid stream length");
		}

		TEST_METHOD(SerializerISerializableTest)
		{
			Serializer serializer;
			TestStream stream;
			ISerializable *data = { new TeamInfo(0xf01, "Test Team") };
			serializer.write(data, 1, &stream);

			Assert::IsTrue(stream.Len > 0, L"Stream should not be empty");

			TeamInfo res;
			for (int i = 1; i < stream.Len; i++) {
				Assert::IsFalse(res.deserialize(&serializer), L"Object should not be deserializable");
				serializer.append(stream.Data[i]);
			}
			Assert::IsTrue(res.deserialize(&serializer), L"Object could not be deserialized");
			Assert::IsTrue((unsigned short)0xf01 == res.id, L"Object deserialized incorrectly");
			Assert::AreEqual("Test Team", res.name, L"Object deserialized incorrectly");
		}

		TEST_METHOD(SerializerArrayTest)
		{
			Serializer serializer;
			TestStream stream;
			Array<PlayerInfo> arr(new PlayerInfo*[] {new PlayerInfo(0x01, 0x100, "PLR1"), new PlayerInfo(0x02, 0x200, "PLR2")}, 2);
			arr.serialize(&serializer, &stream);

			Assert::IsTrue(stream.Len > 0, L"Stream should not be empty");
			Array<PlayerInfo> res;
			for (int i = 0; i < stream.Len; i++) {
				Assert::IsFalse(res.deserialize(&serializer), L"Array should not be deserializable");
				serializer.append(stream.Data[i]);
			}
			Assert::IsTrue(res.deserialize(&serializer), L"Array could not be deserialized");
		}

		TEST_METHOD(SerializerGeneratedObjectTest)
		{
			Serializer serializer;
			TestStream stream;
			GameParameters params;
			params.bases = Array<BaseInfo>(new BaseInfo*[] { new BaseInfo(0xf00, 0xba, 'r') }, 1);
			params.id = "game_test";
			params.players = Array<PlayerInfo>(new PlayerInfo*[] { new PlayerInfo(0xbad, 0x100, "PLR1"), new PlayerInfo(0x10, 0xacdc, "TEST") }, 2);
			params.teams = Array<TeamInfo>(new TeamInfo*[] { new TeamInfo(0x100, "Team One"), new TeamInfo(0xacdc, "Testers") }, 2);
			params.rules = new Rules();
			params.serialize(&serializer, &stream);

			Assert::IsTrue(stream.Len > 0, L"Stream should not be empty");

			GameParameters result;
			for (int i = 0; i < stream.Len; i++) {
				if (result.deserialize(&serializer)) {
					Assert::Fail(L"Deserialization should not have completed");
				}
				serializer.append(stream.Data[i]);
			}
			Assert::IsTrue(result.deserialize(&serializer), L"Deserialization should have completed");

			Assert::AreEqual(params.id, result.id, "Game id incorrectly serialized");

			Assert::AreEqual(params.bases.length(), result.bases.length(), L"Number of bases does not match");
			Assert::IsTrue(params.bases.get(0)->id == result.bases.get(0)->id, L"Base information incorrectly serialized");
			Assert::IsTrue(params.bases.get(0)->state == result.bases.get(0)->state, L"Base information incorrectly serialized");
			Assert::IsTrue(params.bases.get(0)->type == result.bases.get(0)->type, L"Base information incorrectly serialized");

			Assert::AreEqual(params.players.length(), result.players.length(), L"Number of players does not match");
			Assert::IsTrue(params.players.get(0)->id == result.players.get(0)->id, L"Player information incorrectly serialized");
			Assert::AreEqual(params.players.get(0)->name, result.players.get(0)->name, "Player information incorrectly serialized");
			Assert::IsTrue(params.players.get(0)->teamId == result.players.get(0)->teamId, L"Player information incorrectly serialized");
			Assert::IsTrue(params.players.get(1)->id == result.players.get(1)->id, L"Player information incorrectly serialized");
			Assert::AreEqual(params.players.get(1)->name, result.players.get(1)->name, "Player information incorrectly serialized");
			Assert::IsTrue(params.players.get(1)->teamId == result.players.get(1)->teamId, L"Player information incorrectly serialized");

			Assert::AreEqual(params.teams.length(), result.teams.length(), L"Number of teams does not match");
			Assert::IsTrue(params.teams.get(0)->id == result.teams.get(0)->id, L"Team information incorrectly serialized");
			Assert::AreEqual(params.teams.get(0)->name, result.teams.get(0)->name, "Team information incorrectly serialized");
			Assert::IsTrue(params.teams.get(1)->id == result.teams.get(1)->id, L"Team information incorrectly serialized");
			Assert::AreEqual(params.teams.get(1)->name, result.teams.get(1)->name, "Team information incorrectly serialized");
		}
	};
}
