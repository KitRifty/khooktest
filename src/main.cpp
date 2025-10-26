#include "main.hpp"

#include <gtest/gtest.h>

#include <khook.hpp>

NOINLINE void bool_copy(bool* dst, bool* src) {
    *dst = *src;
}

NOINLINE void bool_dtor(bool* ptr) {
    *ptr = false;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    KHook::Shutdown();

    return result;
}