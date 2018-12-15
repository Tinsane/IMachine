extern "C" {
#include "../src/subprocess.h"
#include "../src/code_line.h"
#include "../src/tokenized_reader.h"
}

#include <set>
#include <functional>
#include <string>
#include <cstring>
#include <cstdio>
#include "gtest/gtest.h"

using std::function;
using std::string;
using std::set;

class TestSubprocess : public ::testing::Test {
protected:
    void SetUp() {
        memset(Memory, 0, sizeof Memory);
    }
    void TearDown() {
    }
    uint16_t Memory[MEMORY_SZ];
};

TEST_F(TestSubprocess, TestRunInstruction_TicksOver) {
    auto subprocess = NewSubprocess(Memory, 1, 10);
    subprocess->TicksExecuted = 1;
    EXPECT_FALSE(RunInstruction(subprocess));
    DeleteSubprocess(subprocess);
}

TEST_F(TestSubprocess, TestRunInstruction_UnalignedIP) {
    auto subprocess = NewSubprocess(Memory, 1, 10);
    subprocess->RegisterSet[8] = 1;
    EXPECT_FALSE(RunInstruction(subprocess));
    DeleteSubprocess(subprocess);
}

TEST_F(TestSubprocess, TestRunInstruction_InvalidInstruction) {
    auto subprocess = NewSubprocess(Memory, 1, 10);
    EXPECT_FALSE(RunInstruction(subprocess));
    DeleteSubprocess(subprocess);
}

TEST_F(TestSubprocess, TestRunInstruction_OkInstruction) {
    CodeLine line;
    ResetLine(&line);
    auto subprocess = NewSubprocess(Memory, 2, 10);
    char strInstruction[] = " nop";
    auto reader = TokenizedReaderFromBuffer(strInstruction);
    ASSERT_TRUE(ParseLine(&reader, &line));
    Memory[5] = line.RawInstruction;
    EXPECT_TRUE(RunInstruction(subprocess));
    DeleteSubprocess(subprocess);
}

