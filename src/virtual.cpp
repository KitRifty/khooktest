#include <gtest/gtest.h>

#include <iostream>
#include <khook.hpp>
#include <thread>

#include "main.hpp"

class VirtualHookTest: public ::testing::Test {
  protected:
    class TestObject {};

    class HookedClass {
      public:
        virtual bool IsAllowed(TestObject* obj) {
            std::cout << "TestClass::IsAllowed()" << std::endl;
            return true;
        }

        virtual bool IsForbidden(TestObject* obj) {
            std::cout << "TestClass::IsForbidden()" << std::endl;
            return true;
        }
    };

    class FakeClass {
      public:
        NOINLINE bool TestDetour_Pre(TestObject* obj) {
            std::cout << "TestDetour_Pre()" << std::endl;
            std::cout << "ptr: " << std::hex << this << std::endl;
            std::cout << "obj: " << std::hex << obj << std::endl;
            bool result = false;
            KHook::SaveReturnValue(
                KHook::Action::Override,
                &result,
                sizeof(bool),
                (void*)bool_copy,
                (void*)bool_dtor,
                false
            );
            return false;
        }

        NOINLINE bool TestDetour_CallOriginal(TestObject* obj) {
            std::cout << "TestDetour_CallOriginal()" << std::endl;
            std::cout << "ptr: " << std::hex << this << std::endl;
            std::cout << "obj: " << std::hex << obj << std::endl;
            auto original = KHook::BuildMFP<FakeClass, bool, TestObject*>(
                KHook::GetOriginalFunction()
            );
            bool result = (this->*original)(obj);
            KHook::SaveReturnValue(
                KHook::Action::Ignore,
                &result,
                sizeof(bool),
                (void*)bool_copy,
                (void*)bool_dtor,
                true
            );
            return false;
        }

        NOINLINE bool TestDetour_Post(TestObject* obj) {
            std::cout << "TestDetour_Post()" << std::endl;
            std::cout << "ptr: " << std::hex << this << std::endl;
            std::cout << "obj: " << std::hex << obj << std::endl;
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

        NOINLINE bool TestDetour_MakeReturn(TestObject* obj) {
            std::cout << "TestDetour_MakeReturn()" << std::endl;
            std::cout << "ptr: " << std::hex << this << std::endl;
            std::cout << "obj: " << std::hex << obj << std::endl;
            bool result = *((bool*)KHook::GetCurrentValuePtr(true));
            KHook::DestroyReturnValue();
            return result;
        }

        NOINLINE void TestDetour_OnRemoved(int hookId) {
            std::cout << "TestDetour_OnRemoved()" << std::endl;
            std::cout << "hookId: " << hookId << std::endl;
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
};

TEST_F(VirtualHookTest, AddAndRemoveResult) {
    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::IsAllowed),
        nullptr,
        KHook::ExtractMFP(&FakeClass::TestDetour_OnRemoved),
        KHook::ExtractMFP(&FakeClass::TestDetour_Pre),
        KHook::ExtractMFP(&FakeClass::TestDetour_Post),
        KHook::ExtractMFP(&FakeClass::TestDetour_MakeReturn),
        KHook::ExtractMFP(&FakeClass::TestDetour_CallOriginal),
        true
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    bool result = target->IsAllowed(obj);
    EXPECT_FALSE(result) << "Method should return false when hooked";

    KHook::RemoveHook(hookId, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    result = target->IsAllowed(obj);
    EXPECT_TRUE(result) << "Method should return true after hook removal";
}

TEST_F(VirtualHookTest, HookRemovalCallback) {
    testing::internal::CaptureStdout();

    int hookId = KHook::SetupVirtualHook(
        *(void***)(target),
        KHook::GetVtableIndex(&HookedClass::IsAllowed),
        nullptr,
        KHook::ExtractMFP(&FakeClass::TestDetour_OnRemoved),
        KHook::ExtractMFP(&FakeClass::TestDetour_Pre),
        KHook::ExtractMFP(&FakeClass::TestDetour_Post),
        KHook::ExtractMFP(&FakeClass::TestDetour_MakeReturn),
        KHook::ExtractMFP(&FakeClass::TestDetour_CallOriginal),
        true
    );

    ASSERT_NE(hookId, KHook::INVALID_HOOK) << "Hook setup should succeed";

    KHook::RemoveHook(hookId, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("TestDetour_OnRemoved()") != std::string::npos)
        << "Hook removal callback should be called";
}