#include "stdafx.h"
#include "CppUnitTest.h"
#include <OpenBattleCore.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace OpenBattle;

namespace Tests
{		
	class TestMessage : public Message {
		unsigned short d = 0;
		bool disposed = false;
	public:
		TestMessage() {
			this->d = 0;
		}
		unsigned char version() {
			return 0x02;
		}
		unsigned char type() {
			return 0x01;
		}
		bool deserialize(Serializer *serializer) {
			if (!serializer->read(&this->d)) {
				return false;
			}
			return true;
		}
		void serialize(Serializer *serializer, Stream *stream) {

		}
		void dispose() {
			this->disposed = true;
		}
		unsigned short data() {
			return this->d;
		}
		bool isDisposed() {
			return this->disposed;
		}
	};

	class TestMessageFactory : public MessageFactory
	{
	public:
		unsigned char type() {
			return 0x01;
		}
		Message *get(unsigned char ver) {
			return new TestMessage();
		}
	};

	class TestMessageHandler : public MessageHandler
	{
		unsigned int d;
	public:
		TestMessageHandler() {
			this->d = 0;
		}
		unsigned char type() {
			return 0x01;
		}
		void invoke(Message *msg) {
			if (msg != 0) {
				this->d += ((TestMessage*)msg)->data();
			}
		}
		unsigned int data() {
			return this->d;
		}
	};

	TEST_CLASS(ProtocolTests)
	{
	public:
		
		TEST_METHOD(MessageFactoryTest)
		{
			MessageFactoryCollection *factory = new MessageFactoryCollection();
			factory->add(new TestMessageFactory());
			
			Message *msg = factory->get(0x01, 0x01);
			Assert::IsFalse(msg == 0, L"Message was not initialized");
		}

		TEST_METHOD(ProtocolConsumeTest)
		{
			Protocol *protocol = new Protocol();
			TestMessageHandler *handler = new TestMessageHandler();
			protocol->addFactory(new TestMessageFactory());
			protocol->addHandler(handler);

			unsigned char data[] = { 0x01, 0x01, 0x02, 0xF4, 0x01, 0x01, 0x01, 0x02, 0xF4, 0x01 };
			for (int j = 0; j < 10; j++) {
				for (int i = 0; i < 10; i++) {
					protocol->consume(data[i]);
				}
			}

			Assert::IsTrue(handler->data() == 10000, L"Handler was not invoked");
		}

		TEST_METHOD(ProtocolChannelTest)
		{
			Protocol *protocol = new Protocol(2);
			TestMessageHandler *handler = new TestMessageHandler();
			protocol->addFactory(new TestMessageFactory());
			protocol->addHandler(handler);

			unsigned char data[] = { 0x01, 0x01, 0x02, 0xF4, 0x01 };
			for (int i = 0; i < 5; i++) {
				protocol->consume(data[i], 0);
				protocol->consume(data[i], 1);
			}

			Assert::IsTrue(handler->data() == 1000, L"Handler was not invoked");
		}
	};
}