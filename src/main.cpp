#include <iostream>
#include <cassert>
#include <khook.hpp>

#include "main.hpp"

#if defined(_MSC_VER)
    #define NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    #define NOINLINE __attribute__((noinline))
#else
    #define NOINLINE
#endif

class TestObject {};

class TestClass {
public:
    NOINLINE bool IsAllowed(TestObject* obj) {
        std::cout << "TestClass::IsAllowed()" << std::endl;
        std::cout << "this: " << std::hex << this << std::endl;
        std::cout << "obj: " << std::hex << obj << std::endl;
        return true;
    }
};

NOINLINE void bool_copy(bool* dst, bool* src) {
    *dst = *src;
}

NOINLINE void bool_dtor(bool* ptr) {
    *ptr = false;
}

NOINLINE bool __thiscall TestDetour_Pre(TestClass* ptr, TestObject* obj) {
    std::cout << "TestDetour_Pre()" << std::endl;
    std::cout << "ptr: " << std::hex << ptr << std::endl;
    std::cout << "obj: " << std::hex << obj << std::endl;
    bool result = false;
    KHook::SaveReturnValue(KHook::Action::Override, &result, sizeof(bool), bool_copy, bool_dtor, false);
    return false;
}

NOINLINE bool __thiscall TestDetour_CallOriginal(TestClass* ptr, TestObject* obj) {
    std::cout << "TestDetour_CallOriginal()" << std::endl;
    std::cout << "ptr: " << std::hex << ptr << std::endl;
    std::cout << "obj: " << std::hex << obj << std::endl;
    auto original = (bool(__thiscall*)(TestClass*, TestObject*))KHook::GetOriginalFunction();
    bool result = original(ptr, obj);
    KHook::SaveReturnValue(KHook::Action::Ignore, &result, sizeof(bool), bool_copy, bool_dtor, true);
    return false;
}

NOINLINE bool __thiscall TestDetour_Post(TestClass* ptr, TestObject* obj) {
    std::cout << "TestDetour_Post()" << std::endl;
    std::cout << "ptr: " << std::hex << ptr << std::endl;
    std::cout << "obj: " << std::hex << obj << std::endl;
    KHook::SaveReturnValue(KHook::Action::Ignore, nullptr, 0, nullptr, nullptr, false);
    return false;
}

NOINLINE bool __thiscall TestDetour_MakeReturn(TestClass* ptr, TestObject* obj) {
    std::cout << "TestDetour_MakeReturn()" << std::endl;
    std::cout << "ptr: " << std::hex << ptr << std::endl;
    std::cout << "obj: " << std::hex << obj << std::endl;
    bool result = *((bool*)KHook::GetCurrentValuePtr(true));
    KHook::DestroyReturnValue();
    return result;
}

NOINLINE void TestDetour_OnRemoved(int hookId)
{
}

int main() {
    int hookId = KHook::SetupHook(
        KHook::ExtractMFP(&TestClass::IsAllowed),
        nullptr,
        &TestDetour_OnRemoved,
        &TestDetour_Pre,
        &TestDetour_Post,
        &TestDetour_MakeReturn,
        &TestDetour_CallOriginal,
        true);

    assert(hookId != KHook::INVALID_HOOK);

    std::cout << "hook id: " << std::dec << hookId << std::endl;

    TestClass* target = new TestClass();
    TestObject* obj = new TestObject();

    std::cout << "target: " << std::hex << target << std::endl;
    std::cout << "obj: " << std::hex << obj << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    bool result = target->IsAllowed(obj);

    std::cout << "returned: " << result << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    KHook::RemoveHook(hookId, true);

    std::cout << "hook removed" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "target: " << std::hex << target << std::endl;
    std::cout << "obj: " << std::hex << obj << std::endl;

    result = target->IsAllowed(obj);
    std::cout << "returned after unhook: " << result << std::endl;

    delete obj;
    delete target;

    KHook::Shutdown();

    return 0;
}