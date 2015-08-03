#include "stdafx.h"
#include "CppUnitTest.h"

#include <OpenBattleCore.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace OpenBattle;

int callback = 0;

namespace Tests
{
	TEST_CLASS(ScriptTests)
	{
	public:
		TEST_METHOD(ScriptUpdateTest)
		{
			class Context {
			public:
				static void Callback() {
					callback++;
				}
			};

			Script script(new Step*[] { new Step(100, &Context::Callback), new Step(200, &Context::Callback) }, 2);
			ScriptRunner runner;
			runner.setScript(&script);
			runner.update(50UL);

			Assert::AreEqual((unsigned char)0, script.step(), L"Script should not have advanced");
			Assert::AreEqual(1, callback, L"Callback should have been called");

			runner.update(100UL);
			Assert::AreEqual((unsigned char)1, script.step(), L"Script should not have advanced");
			Assert::AreEqual(2, callback, L"Callback should have been called");

			runner.update(350UL);
			Assert::AreEqual((unsigned char)0, script.step(), L"Script should not have advanced");
			Assert::AreEqual(3, callback, L"Callback should have been called");

			runner.update(360UL);
			Assert::AreEqual((unsigned char)0, script.step(), L"Script should not have advanced");
			Assert::AreEqual(3, callback, L"Callback should NOT have been called");
		}

	};
}