extern "C" {
#include "../src/instruction_set.h"
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

TEST_F(TestNonJumpInstructions, TestNop) {
    auto outer_this = this;
    TestInstruction(" nop",
        [](uint16_t[], uint16_t[]){
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            outer_this->CheckStateExcept({8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestAdd) {
    auto outer_this = this;
    TestInstruction(" add R1X R2X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 168);
            EXPECT_EQ(Registers[2], 123);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestMul) {
    auto outer_this = this;
    TestInstruction(" mul R1X R2X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 5535);
            EXPECT_EQ(Registers[2], 123);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestSub) {
    auto outer_this = this;
    TestInstruction(" sub R1X R2X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 65458);
            EXPECT_EQ(Registers[2], 123);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestDiv) {
    auto outer_this = this;
    TestInstruction(" div R2X R1X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 45);
            EXPECT_EQ(Registers[2], 2);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestAnd) {
    auto outer_this = this;
    TestInstruction(" and R1X R2X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 41);
            EXPECT_EQ(Registers[2], 123);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestOr) {
    auto outer_this = this;
    TestInstruction(" or R1X R2X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 127);
            EXPECT_EQ(Registers[2], 123);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestXor) {
    auto outer_this = this;
    TestInstruction(" xor R1X R2X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 86);
            EXPECT_EQ(Registers[2], 123);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestNot) {
    auto outer_this = this;
    TestInstruction(" not R1X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 65490);
            outer_this->CheckStateExcept({1, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestShl) {
    auto outer_this = this;
    TestInstruction(" shl R1X #3",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 360);
            outer_this->CheckStateExcept({1, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestShr) {
    auto outer_this = this;
    TestInstruction(" shr R1X #3",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[1] = 45;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 5);
            outer_this->CheckStateExcept({1, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestMovv) {
    auto outer_this = this;
    TestInstruction(" mov R1H #12",
        [](uint16_t Registers[], uint16_t Memory[]){
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 3072);
            outer_this->CheckStateExcept({1, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestMovr) {
    auto outer_this = this;
    TestInstruction(" mov R1X R2X",
        [](uint16_t Registers[], uint16_t Memory[]){
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 123);
            EXPECT_EQ(Registers[2], 123);
            outer_this->CheckStateExcept({1, 2, 8});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestMov1) {
    auto outer_this = this;
    TestInstruction(" mov (R1X) R2X",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[1] = 22;
            Registers[2] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 22);
            EXPECT_EQ(Registers[2], 123);
            EXPECT_EQ(Memory[11], 123);
            outer_this->CheckStateExcept({1, 2, 8}, {11});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestMov2) {
    auto outer_this = this;
    TestInstruction(" mov R2X (R1X)",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[1] = 22;
            Memory[11] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 22);
            EXPECT_EQ(Registers[2], 123);
            EXPECT_EQ(Memory[11], 123);
            outer_this->CheckStateExcept({1, 2, 8}, {11});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestPush) {
    auto outer_this = this;
    TestInstruction(" push R1X",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[1] = 22;
            Registers[7] = 10;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 22);
            EXPECT_EQ(Registers[7], 8);
            EXPECT_EQ(Memory[4], 22);
            outer_this->CheckStateExcept({1, 7, 8}, {4});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestPop) {
    auto outer_this = this;
    TestInstruction(" pop R1X",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[7] = 10;
            Memory[5] = 123;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 123);
            EXPECT_EQ(Registers[7], 12);
            EXPECT_EQ(Memory[5], 123);
            outer_this->CheckStateExcept({1, 7, 8}, {5});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestCall) {
    auto outer_this = this;
    TestInstruction(" call #13",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[7] = 10;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[7], 8);
            EXPECT_EQ(Memory[4], 2);
            outer_this->CheckStateExcept({7, 8}, {4});
            EXPECT_EQ(Registers[8], 26);
    });
}

TEST_F(TestNonJumpInstructions, TestRet) {
    auto outer_this = this;
    TestInstruction(" ret",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[7] = 10;
            Memory[5] = 34;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[7], 12);
            EXPECT_EQ(Memory[5], 34);
            outer_this->CheckStateExcept({7, 8}, {5});
            EXPECT_EQ(Registers[8], 34);
    });
}

TEST_F(TestNonJumpInstructions, TestCmpxchg) {
    auto outer_this = this;
    TestInstruction(" cmpxchg (R1X) R2X R3X",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[1] = 2;
            Memory[1] = 3;
            Registers[2] = 3;
            Registers[3] = 4;
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 2);
            EXPECT_EQ(Memory[1], 4);
            EXPECT_EQ(Registers[2], 3);
            EXPECT_EQ(Registers[3], 3);
            outer_this->CheckStateExcept({1, 2, 3, 8}, {1});
            EXPECT_EQ(Registers[8], 2);
    });
}

TEST_F(TestNonJumpInstructions, TestIn) {
    auto outer_this = this;
    char stdinBuf[] = "x";
    auto realStdin = stdin;
    stdin = fmemopen(stdinBuf, 1, "r");
    TestInstruction(" in R1L",
        [](uint16_t Registers[], uint16_t Memory[]) {
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 'x');
            outer_this->CheckStateExcept({1, 8});
            EXPECT_EQ(Registers[8], 2);
    });
    stdin = realStdin;
}

TEST_F(TestNonJumpInstructions, TestOut) {
    auto outer_this = this;
    char stdoutBuf[3];
    auto realStdout = stdout;
    stdout = fmemopen(stdoutBuf, 1, "w");
    TestInstruction(" out R1L",
        [](uint16_t Registers[], uint16_t Memory[]) {
            Registers[1] = 'x';
        }, [outer_this](uint16_t Registers[], uint16_t Memory[]) {
            EXPECT_EQ(Registers[1], 'x');
            outer_this->CheckStateExcept({1, 8});
            EXPECT_EQ(Registers[8], 2);
    });
    stdout = realStdout;
}

TEST_F(TestNonJumpInstructions, TestReset) {
    char buffer[] = " reset";
    auto reader = TokenizedReaderFromBuffer(buffer);
    ASSERT_TRUE(ParseLine(&reader, &line));
    Registers[0] = 4;
    auto instructionId = GetInstructionId(line.RawInstruction);
    EXPECT_FALSE(INSTRUCTIONS[instructionId](line.RawInstruction, Registers, Memory));
    EXPECT_EQ(InstructionSet_ExitCode, 4);
}

