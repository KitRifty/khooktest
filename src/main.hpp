#pragma once

#include <iostream>
#include <khook.hpp>
#include <type_traits>

#if defined(_MSC_VER)
    #define NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    #define NOINLINE __attribute__((noinline))
    #define __thiscall
#else
    #define NOINLINE
#endif

#pragma region StaticHookTemplate

template<typename Ret, typename... Args>
class NoopStaticHookTemplate {
  public:
    static NOINLINE Ret PrePostNoop(Args... args);
    static NOINLINE Ret CallOriginal(Args... args);
    static NOINLINE Ret MakeReturn(Args... args);
    static NOINLINE void OnRemoved(int hookId);
};

template<typename Ret, typename... Args>
NOINLINE Ret NoopStaticHookTemplate<Ret, Args...>::PrePostNoop(Args... args) {
    std::cout << "PrePostNoop()" << std::endl;
    KHook::SaveReturnValue(
        KHook::Action::Ignore,
        nullptr,
        0,
        nullptr,
        nullptr,
        false
    );
    if constexpr (std::is_same<Ret, void>::value) {
        return;
    } else {
        return Ret();
    }
}

template<typename Ret, typename... Args>
NOINLINE Ret NoopStaticHookTemplate<Ret, Args...>::CallOriginal(Args... args) {
    std::cout << "CallOriginal()" << std::endl;
    auto original =
        reinterpret_cast<Ret (*)(Args...)>(KHook::GetOriginalFunction());
    if constexpr (std::is_same<Ret, void>::value) {
        original(args...);
        KHook::SaveReturnValue(
            KHook::Action::Ignore,
            nullptr,
            0,
            nullptr,
            nullptr,
            true
        );
        return;
    } else {
        Ret result = original(args...);
        KHook::SaveReturnValue(
            KHook::Action::Ignore,
            &result,
            sizeof(Ret),
            (void*)KHook::init_operator<Ret>,
            (void*)KHook::deinit_operator<Ret>,
            true
        );
        return result;
    }
}

template<typename Ret, typename... Args>
NOINLINE Ret NoopStaticHookTemplate<Ret, Args...>::MakeReturn(Args... args) {
    std::cout << "MakeReturn()" << std::endl;
    if constexpr (std::is_same<Ret, void>::value) {
        KHook::DestroyReturnValue();
        return;
    } else {
        Ret result = *((Ret*)KHook::GetCurrentValuePtr(true));
        KHook::DestroyReturnValue();
        return result;
    }
}

template<typename Ret, typename... Args>
NOINLINE void NoopStaticHookTemplate<Ret, Args...>::OnRemoved(int hookId) {
    std::cout << "OnRemoved(" << std::dec << hookId << ")" << std::endl;
}

#pragma endregion

#pragma region MemberHookTemplate

template<typename Ret, typename... Args>
class NoopMemberHookTemplate {
  public:
    NOINLINE Ret PrePostNoop(Args... args);
    NOINLINE Ret CallOriginal(Args... args);
    NOINLINE Ret MakeReturn(Args... args);
    NOINLINE void OnRemoved(int hookId);
};

template<typename Ret, typename... Args>
NOINLINE Ret NoopMemberHookTemplate<Ret, Args...>::PrePostNoop(Args... args) {
    std::cout << "PrePostNoop()" << std::endl;
    KHook::SaveReturnValue(
        KHook::Action::Ignore,
        nullptr,
        0,
        nullptr,
        nullptr,
        false
    );
    if constexpr (std::is_same<Ret, void>::value) {
        return;
    } else {
        return Ret();
    }
}

template<typename Ret, typename... Args>
NOINLINE Ret NoopMemberHookTemplate<Ret, Args...>::CallOriginal(Args... args) {
    std::cout << "CallOriginal()" << std::endl;
    auto original = reinterpret_cast<Ret(__thiscall*)(void*, Args...)>(
        KHook::GetOriginalFunction()
    );
    if constexpr (std::is_same<Ret, void>::value) {
        original(this, args...);
        KHook::SaveReturnValue(
            KHook::Action::Ignore,
            nullptr,
            0,
            nullptr,
            nullptr,
            true
        );
        return;
    } else {
        Ret result = original(this, args...);
        KHook::SaveReturnValue(
            KHook::Action::Ignore,
            &result,
            sizeof(Ret),
            (void*)KHook::init_operator<Ret>,
            (void*)KHook::deinit_operator<Ret>,
            true
        );
        return result;
    }
}

template<typename Ret, typename... Args>
NOINLINE Ret NoopMemberHookTemplate<Ret, Args...>::MakeReturn(Args... args) {
    std::cout << "MakeReturn()" << std::endl;
    if constexpr (std::is_same<Ret, void>::value) {
        KHook::DestroyReturnValue();
        return;
    } else {
        Ret result = *((Ret*)KHook::GetCurrentValuePtr(true));
        KHook::DestroyReturnValue();
        return result;
    }
}

template<typename Ret, typename... Args>
NOINLINE void NoopMemberHookTemplate<Ret, Args...>::OnRemoved(int hookId) {
    std::cout << "OnRemoved(" << std::dec << hookId << ")" << std::endl;
}

#pragma endregion