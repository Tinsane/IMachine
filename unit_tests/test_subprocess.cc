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

class TestNonJumpInstructions : public ::testing::Test {
protected:
    void SetUp() {
        memset(Memory, 0, sizeof Memory);
        memset(Registers, 0, sizeof Registers);
        ResetLine(&line);
    }
    void TearDown() {
    }
    uint16_t Memory[MEMORY_SZ];
    uint16_t Registers[REGISTER_CNT];
    CodeLine line;

    void TestInstruction(
            string strInstruction,
            function<void(uint16_t[], uint16_t[])> setUp,
            function<void(uint16_t[], uint16_t[])> check) {
        setUp(Registers, Memory);
        auto reader = TokenizedReaderFromBuffer(strInstruction.c_str());
        ASSERT_TRUE(ParseLine(&reader, &line));
        auto instructionId = GetInstructionId(line.RawInstruction);
        EXPECT_TRUE(INSTRUCTIONS[instructionId](line.RawInstruction, Registers, Memory));
        check(Registers, Memory);
    }

    void CheckStateExcept(set<size_t> changedRegs, set<size_t> changedMemoryCells = {}) {
        for (size_t i = 0; i < MEMORY_SZ; ++i) {
            if (!changedMemoryCells.count(i)) {
                EXPECT_EQ(Memory[i], 0);
            }
        }
        for (size_t i = 0; i < REGISTER_CNT; ++i) {
            if (!changedRegs.count(i)) {
                EXPECT_EQ(Registers[i], 0);
            }
        }
    }

};

