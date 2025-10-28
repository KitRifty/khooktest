#include <gtest/gtest.h>

#include <iostream>
#include <khook.hpp>
#include <thread>

#include "main.hpp"

class VirtualHookTest: public ::testing::Test {
  protected:
    class TestObject {
      public:
        int m_testValue;
    };

    class HookedClass {
      public:
        virtual bool IsAllowed(TestObject* obj) {
            std::cout << "HookedClass::IsAllowed()" << std::endl;
            return true;
        }

        virtual int SetObjectValue(TestObject* obj, int value) {
            std::cout << "HookedClass::SetObjectValue()" << std::endl;
            obj->m_testValue = value;
            return value;
        }

        virtual void MyVoid(TestObject* obj) {
            std::cout << "HookedClass::MyVoid()" << std::endl;
        }
    };

    class IsAllowedHook {
      public:
        NOINLINE bool PrePostNoop(TestObject* obj) {
            std::cout << "PrePostNoop()" << std::endl;
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                nullptr,
                0,
                nullptr,
                nullptr,
                false
            );
            return false;
        }

        NOINLINE bool CallOriginal(TestObject* obj) {
            std::cout << "CallOriginal()" << std::endl;
            auto original = KHook::BuildMFP<IsAllowedHook, bool, TestObject*>(
                KHook::GetOriginalFunction()
            );
            bool result = (this->*original)(obj);
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                &result,
                sizeof(bool),
                (void*)KHook::init_operator<bool>,
                (void*)KHook::deinit_operator<bool>,
                true
            );
            return false;
        }

        NOINLINE bool MakeReturn(TestObject* obj) {
            std::cout << "MakeReturn()" << std::endl;
            bool result = *((bool*)KHook::GetCurrentValuePtr(true));
            KHook::DestroyReturnValue();
            return result;
        }

        NOINLINE void OnRemoved(int hookId) {
            std::cout << "OnRemoved(" << std::dec << hookId << ")" << std::endl;
        }
    };

    class SetObjectValueHook {
      public:
        NOINLINE int PrePostNoop(TestObject* obj, int value) {
            std::cout << "PrePostNoop()" << std::endl;
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                nullptr,
                0,
                nullptr,
                nullptr,
                false
            );
            return 0;
        }

        NOINLINE int CallOriginal(TestObject* obj, int value) {
            std::cout << "CallOriginal()" << std::endl;
            auto original =
                KHook::BuildMFP<SetObjectValueHook, int, TestObject*, int>(
                    KHook::GetOriginalFunction()
                );
            int result = (this->*original)(obj, value);
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                &result,
                sizeof(int),
                (void*)KHook::init_operator<int>,
                (void*)KHook::deinit_operator<int>,
                true
            );
            return 0;
        }

        NOINLINE int MakeReturn(TestObject* obj, int value) {
            std::cout << "MakeReturn()" << std::endl;
            int result = *((int*)KHook::GetCurrentValuePtr(true));
            KHook::DestroyReturnValue();
            return result;
        }

        NOINLINE void OnRemoved(int hookId) {
            std::cout << "OnRemoved(" << std::dec << hookId << ")" << std::endl;
        }
    };

    class MyVoidHook {
      public:
        NOINLINE void PrePostNoop(TestObject* obj) {
            std::cout << "PrePostNoop()" << std::endl;
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                nullptr,
                0,
                nullptr,
                nullptr,
                false
            );
        }

        NOINLINE void CallOriginal(TestObject* obj) {
            std::cout << "CallOriginal()" << std::endl;
            auto original = KHook::BuildMFP<MyVoidHook, void, TestObject*>(
                KHook::GetOriginalFunction()
            );
            (this->*original)(obj);
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                nullptr,
                0,
                nullptr,
                nullptr,
                true
            );
        }

        NOINLINE void MakeReturn(TestObject* obj) {
            std::cout << "MakeReturn()" << std::endl;
            KHook::DestroyReturnValue();
        }

        NOINLINE void OnRemoved(int hookId) {
            std::cout << "OnRemoved(" << std::dec << hookId << ")" << std::endl;
        }
    };

    class FakeClass {
      public:
        NOINLINE bool OverrideIsAllowedReturnValue(TestObject* obj) {
            std::cout << "OverrideReturnValue()" << std::endl;
            bool result = false;
            KHook::SaveReturnValue(
                KHook::Action::Override,
                &result,
                sizeof(bool),
                (void*)KHook::init_operator<bool>,
                (void*)KHook::deinit_operator<bool>,
                false
            );
            return false;
        }

        NOINLINE bool SupersedeIsAllowedReturnValue(TestObject* obj) {
            std::cout << "SupersedeReturnValue()" << std::endl;
            bool result = false;
            KHook::SaveReturnValue(
                KHook::Action::Supersede,
                &result,
                sizeof(bool),
                (void*)KHook::init_operator<bool>,
                (void*)KHook::deinit_operator<bool>,
                false
            );
            return false;
        }

        NOINLINE int OverrideSetObjectValue(TestObject* obj, int value) {
            auto recall = KHook::BuildMFP<FakeClass, int, TestObject*, int>(
                KHook::DoRecall(
                    KHook::Action::Ignore,
                    nullptr,
                    0,
                    nullptr,
                    nullptr
                )
            );
            (this->*recall)(obj, 1337);
            return 0;
        }

        NOINLINE int SupersedeSetObjectValue(TestObject* obj, int value) {
            std::cout << "SupersedeSetObjectValue()" << std::endl;
            int newValue = 9001;
            KHook::SaveReturnValue(
                KHook::Action::Supersede,
                &newValue,
                sizeof(int),
                (void*)KHook::init_operator<int>,
                (void*)KHook::deinit_operator<int>,
                false
            );
            return 0;
        }

        NOINLINE int HookInsideSetObjectValue(TestObject* obj, int value) {
            m_hookId = KHook::SetupVirtualHook(
                *(void***)(this),
                KHook::GetVtableIndex(&HookedClass::IsAllowed),
                nullptr,
                KHook::ExtractMFP(&IsAllowedHook::OnRemoved),
                KHook::ExtractMFP(&IsAllowedHook::PrePostNoop),
                KHook::ExtractMFP(&IsAllowedHook::PrePostNoop),
                KHook::ExtractMFP(&IsAllowedHook::MakeReturn),
                KHook::ExtractMFP(&IsAllowedHook::CallOriginal),
                false
            );
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                nullptr,
                0,
                nullptr,
                nullptr,
                false
            );
            return 0;
        }

        NOINLINE void SupersedeMyVoid(TestObject* obj) {
            std::cout << "SupersedeMyVoid()" << std::endl;
            KHook::SaveReturnValue(
                KHook::Action::Supersede,
                nullptr,
                0,
                nullptr,
                nullptr,
                false
            );
        }
    };

  protected:
    void SetUp() override {
        target = new HookedClass();
        obj = new TestObject();
    }

    void TearDown() override {
        if (obj) {
            delete obj;
            obj = nullptr;
        }
        if (target) {
            delete target;
            target = nullptr;
        }
    }

    HookedClass* target = nullptr;
    TestObject* obj = nullptr;
    static int m_hookId;
};

int VirtualHookTest::m_hookId = KHook::INVALID_HOOK;

TEST_F(VirtualHookTest, Noop) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::IsAllowed),
        nullptr,
        KHook::ExtractMFP(&IsAllowedHook::OnRemoved),
        KHook::ExtractMFP(&IsAllowedHook::PrePostNoop),
        KHook::ExtractMFP(&IsAllowedHook::PrePostNoop),
        KHook::ExtractMFP(&IsAllowedHook::MakeReturn),
        KHook::ExtractMFP(&IsAllowedHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    testing::internal::CaptureStdout();

    bool overriddenResult = target->IsAllowed(obj);

    KHook::RemoveHook(hookId, false);

    bool originalResult = target->IsAllowed(obj);

    std::string output = testing::internal::GetCapturedStdout();

    std::ostringstream expected;
    expected << "PrePostNoop()" << std::endl;
    expected << "CallOriginal()" << std::endl;
    expected << "HookedClass::IsAllowed()" << std::endl;
    expected << "PrePostNoop()" << std::endl;
    expected << "MakeReturn()" << std::endl;
    expected << "OnRemoved(" << std::dec << hookId << ")" << std::endl;
    expected << "HookedClass::IsAllowed()" << std::endl;

    EXPECT_EQ(output, expected.str())
        << "Callback functions should be called in the expected order";
    EXPECT_TRUE(overriddenResult)
        << "Method should return original value when hooked";
    EXPECT_TRUE(originalResult)
        << "Method should return original value after hook removal";
}

TEST_F(VirtualHookTest, NoopVoid) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::MyVoid),
        nullptr,
        KHook::ExtractMFP(&MyVoidHook::OnRemoved),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::MakeReturn),
        KHook::ExtractMFP(&MyVoidHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    testing::internal::CaptureStdout();

    target->MyVoid(obj);

    KHook::RemoveHook(hookId, false);

    target->MyVoid(obj);

    std::string output = testing::internal::GetCapturedStdout();

    std::ostringstream expected;
    expected << "PrePostNoop()" << std::endl;
    expected << "CallOriginal()" << std::endl;
    expected << "HookedClass::MyVoid()" << std::endl;
    expected << "PrePostNoop()" << std::endl;
    expected << "MakeReturn()" << std::endl;
    expected << "OnRemoved(" << std::dec << hookId << ")" << std::endl;
    expected << "HookedClass::MyVoid()" << std::endl;

    ASSERT_EQ(output, expected.str())
        << "Callback functions should be called in the expected order";
}

TEST_F(VirtualHookTest, OverrideReturnValuePre) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::IsAllowed),
        nullptr,
        KHook::ExtractMFP(&IsAllowedHook::OnRemoved),
        KHook::ExtractMFP(&FakeClass::OverrideIsAllowedReturnValue),
        KHook::ExtractMFP(&IsAllowedHook::PrePostNoop),
        KHook::ExtractMFP(&IsAllowedHook::MakeReturn),
        KHook::ExtractMFP(&IsAllowedHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    bool result = target->IsAllowed(obj);
    EXPECT_FALSE(result) << "Method should return false when hooked";

    KHook::RemoveHook(hookId, false);

    result = target->IsAllowed(obj);
    EXPECT_TRUE(result) << "Method should return true after hook removal";
}

TEST_F(VirtualHookTest, OverrideReturnValuePost) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::IsAllowed),
        nullptr,
        KHook::ExtractMFP(&IsAllowedHook::OnRemoved),
        KHook::ExtractMFP(&IsAllowedHook::PrePostNoop),
        KHook::ExtractMFP(&FakeClass::OverrideIsAllowedReturnValue),
        KHook::ExtractMFP(&IsAllowedHook::MakeReturn),
        KHook::ExtractMFP(&IsAllowedHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    bool result = target->IsAllowed(obj);
    EXPECT_FALSE(result) << "Method should return false when hooked";

    KHook::RemoveHook(hookId, false);

    result = target->IsAllowed(obj);
    EXPECT_TRUE(result) << "Method should return true after hook removal";
}

TEST_F(VirtualHookTest, SupersedeReturnValue) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::IsAllowed),
        nullptr,
        KHook::ExtractMFP(&IsAllowedHook::OnRemoved),
        KHook::ExtractMFP(&FakeClass::SupersedeIsAllowedReturnValue),
        KHook::ExtractMFP(&IsAllowedHook::PrePostNoop),
        KHook::ExtractMFP(&IsAllowedHook::MakeReturn),
        KHook::ExtractMFP(&IsAllowedHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    testing::internal::CaptureStdout();

    bool overriddenResult = target->IsAllowed(obj);

    KHook::RemoveHook(hookId, false);

    bool originalResult = target->IsAllowed(obj);

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output.find("CallOriginal()"), std::string::npos)
        << "Original method should not be called";
    EXPECT_FALSE(overriddenResult) << "Method should return false when hooked";
    EXPECT_TRUE(originalResult)
        << "Method should return true after hook removal";
}

TEST_F(VirtualHookTest, SupersedeVoidReturnValue) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::MyVoid),
        nullptr,
        KHook::ExtractMFP(&MyVoidHook::OnRemoved),
        KHook::ExtractMFP(&FakeClass::SupersedeMyVoid),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::MakeReturn),
        KHook::ExtractMFP(&MyVoidHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    testing::internal::CaptureStdout();

    target->MyVoid(obj);

    std::string output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "SupersedeMyVoid()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "MakeReturn()" << std::endl;
        ASSERT_EQ(expected.str(), output)
            << "Callbacks should be called in the correct order";
    }

    KHook::RemoveHook(hookId, false);

    testing::internal::CaptureStdout();

    target->MyVoid(obj);

    output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "HookedClass::MyVoid()" << std::endl;
        ASSERT_EQ(expected.str(), output)
            << "No callbacks should be called after hook removal";
    }
}

TEST_F(VirtualHookTest, OverrideParameterWithRecall) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::SetObjectValue),
        nullptr,
        KHook::ExtractMFP(&SetObjectValueHook::OnRemoved),
        KHook::ExtractMFP(&FakeClass::OverrideSetObjectValue),
        KHook::ExtractMFP(&SetObjectValueHook::PrePostNoop),
        KHook::ExtractMFP(&SetObjectValueHook::MakeReturn),
        KHook::ExtractMFP(&SetObjectValueHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    int result = target->SetObjectValue(obj, 0xDEADBEEF);
    EXPECT_EQ(obj->m_testValue, 1337)
        << "Method should set value to hooked value";
    EXPECT_EQ(result, 1337) << "Method should return hooked value";

    KHook::RemoveHook(hookId, false);

    result = target->SetObjectValue(obj, 0xDEADBEEF);
    EXPECT_EQ(obj->m_testValue, 0xDEADBEEF)
        << "Method should set value to original value after hook removal";
    EXPECT_EQ(result, 0xDEADBEEF)
        << "Method should return original value after hook removal";
}

TEST_F(VirtualHookTest, SupersedeThenNoopVoidHooksOnSameIndex) {
    int firstHookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::MyVoid),
        nullptr,
        KHook::ExtractMFP(&MyVoidHook::OnRemoved),
        KHook::ExtractMFP(&FakeClass::SupersedeMyVoid),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::MakeReturn),
        KHook::ExtractMFP(&MyVoidHook::CallOriginal),
        false
    );

    ASSERT_NE(firstHookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    int secondHookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::MyVoid),
        nullptr,
        KHook::ExtractMFP(&MyVoidHook::OnRemoved),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::MakeReturn),
        KHook::ExtractMFP(&MyVoidHook::CallOriginal),
        false
    );

    ASSERT_NE(secondHookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    testing::internal::CaptureStdout();
    target->MyVoid(obj);
    std::string output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "PrePostNoop()" << std::endl;
        expected << "SupersedeMyVoid()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "MakeReturn()" << std::endl;

        ASSERT_EQ(expected.str(), output)
            << "Callback functions should be called in the correct order";
    }

    KHook::RemoveHook(firstHookId, false);

    testing::internal::CaptureStdout();
    target->MyVoid(obj);
    output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "PrePostNoop()" << std::endl;
        expected << "CallOriginal()" << std::endl;
        expected << "HookedClass::MyVoid()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "MakeReturn()" << std::endl;

        ASSERT_EQ(expected.str(), output)
            << "Callback functions should be called in the correct order";
    }
}

TEST_F(VirtualHookTest, MultipleNoopVoidHooksOnSameIndex) {
    int firstHookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::MyVoid),
        nullptr,
        KHook::ExtractMFP(&MyVoidHook::OnRemoved),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::MakeReturn),
        KHook::ExtractMFP(&MyVoidHook::CallOriginal),
        false
    );

    ASSERT_NE(firstHookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    int secondHookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::MyVoid),
        nullptr,
        KHook::ExtractMFP(&MyVoidHook::OnRemoved),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::PrePostNoop),
        KHook::ExtractMFP(&MyVoidHook::MakeReturn),
        KHook::ExtractMFP(&MyVoidHook::CallOriginal),
        false
    );

    ASSERT_NE(secondHookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    testing::internal::CaptureStdout();
    target->MyVoid(obj);
    std::string output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "PrePostNoop()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "CallOriginal()" << std::endl;
        expected << "HookedClass::MyVoid()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "MakeReturn()" << std::endl;

        ASSERT_EQ(expected.str(), output)
            << "Callback functions should be called in the correct order";
    }

    KHook::RemoveHook(firstHookId, false);

    testing::internal::CaptureStdout();
    target->MyVoid(obj);
    output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "PrePostNoop()" << std::endl;
        expected << "CallOriginal()" << std::endl;
        expected << "HookedClass::MyVoid()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "MakeReturn()" << std::endl;

        ASSERT_EQ(expected.str(), output)
            << "Callback functions should be called in the correct order";
    }
}

TEST_F(VirtualHookTest, SupersedeThenNoopHooksOnSameIndex) {
    int firstHookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::SetObjectValue),
        nullptr,
        KHook::ExtractMFP(&SetObjectValueHook::OnRemoved),
        KHook::ExtractMFP(&FakeClass::SupersedeSetObjectValue),
        KHook::ExtractMFP(&SetObjectValueHook::PrePostNoop),
        KHook::ExtractMFP(&SetObjectValueHook::MakeReturn),
        KHook::ExtractMFP(&SetObjectValueHook::CallOriginal),
        false
    );

    ASSERT_NE(firstHookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    int secondHookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::SetObjectValue),
        nullptr,
        KHook::ExtractMFP(&SetObjectValueHook::OnRemoved),
        KHook::ExtractMFP(&SetObjectValueHook::PrePostNoop),
        KHook::ExtractMFP(&SetObjectValueHook::PrePostNoop),
        KHook::ExtractMFP(&SetObjectValueHook::MakeReturn),
        KHook::ExtractMFP(&SetObjectValueHook::CallOriginal),
        false
    );

    ASSERT_NE(secondHookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    obj->m_testValue = 0x9600;

    testing::internal::CaptureStdout();
    int result = target->SetObjectValue(obj, 0xDEADBEEF);
    std::string output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "PrePostNoop()" << std::endl;
        expected << "SupersedeSetObjectValue()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "MakeReturn()" << std::endl;

        ASSERT_EQ(expected.str(), output)
            << "Callback functions should be called in the correct order";
    }
    
    KHook::RemoveHook(firstHookId, false);

    obj->m_testValue = 0x9600;

    testing::internal::CaptureStdout();
    result = target->SetObjectValue(obj, 0xDEADBEEF);
    output = testing::internal::GetCapturedStdout();

    {
        std::ostringstream expected;
        expected << "PrePostNoop()" << std::endl;
        expected << "CallOriginal()" << std::endl;
        expected << "HookedClass::SetObjectValue()" << std::endl;
        expected << "PrePostNoop()" << std::endl;
        expected << "MakeReturn()" << std::endl;

        ASSERT_EQ(expected.str(), output)
            << "Callback functions should be called in the correct order";
    }

    KHook::RemoveHook(secondHookId, false);

    obj->m_testValue = 0x9600;

    testing::internal::CaptureStdout();
    result = target->SetObjectValue(obj, 0xDEADBEEF);
    output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output.find("CallOriginal()"), std::string::npos)
        << "CallOriginal() should not be called after all hooks removed";
    EXPECT_EQ(obj->m_testValue, 0xDEADBEEF)
        << "Method should set value to original value after recall hook "
           "removal";
    EXPECT_EQ(result, 0xDEADBEEF)
        << "Method should return original value after recall hook removal";
}

TEST_F(VirtualHookTest, HookIsAllowedInsideSetObjectValue) {
    m_hookId = KHook::INVALID_HOOK;

    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::SetObjectValue),
        nullptr,
        KHook::ExtractMFP(&SetObjectValueHook::OnRemoved),
        KHook::ExtractMFP(&FakeClass::HookInsideSetObjectValue),
        KHook::ExtractMFP(&SetObjectValueHook::PrePostNoop),
        KHook::ExtractMFP(&SetObjectValueHook::MakeReturn),
        KHook::ExtractMFP(&SetObjectValueHook::CallOriginal),
        false
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    int firstResult = target->SetObjectValue(obj, 42);

    EXPECT_EQ(obj->m_testValue, 42)
        << "SetObjectValue should set value to 42 (original behavior)";
    EXPECT_EQ(firstResult, 42)
        << "SetObjectValue should return 42 (original value)";
    ASSERT_NE(m_hookId, KHook::INVALID_HOOK)
        << "IsAllowed hook should have been set up inside SetObjectValue";

    bool result = target->IsAllowed(obj);
    EXPECT_TRUE(result) << "IsAllowed should return true (original value)";
}