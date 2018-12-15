extern "C" {
#include "../src/instruction_set.h"
}

#include "gtest/gtest.h"

class TestInstructionSet : public ::testing::Test {
};

TEST_F(TestInstructionSet, TestGetInstructionId_BigInstruction) {
    uint16_t instructionId = GetInstructionId(BIG_INSTRUCTION_MARKER + (3 << 12));
    EXPECT_EQ(instructionId, 3);
}

TEST_F(TestInstructionSet, TestGetInstructionId_SmallInstruction) {
    uint16_t instructionId = GetInstructionId((21 << 8));
    EXPECT_EQ(instructionId, 21);
}

TEST_F(TestInstructionSet, TestGetInstructionId_InvalidBigInstruction) {
    uint16_t instructionId = GetInstructionId(BIG_INSTRUCTION_MARKER + (5 << 12));
    EXPECT_EQ(instructionId, INVALID_INSTRUCTION_ID);
}

TEST_F(TestInstructionSet, TestGetInstructionId_InvalidSmallInstruction) {
    uint16_t instructionId = GetInstructionId((2 << 8));
    EXPECT_EQ(instructionId, INVALID_INSTRUCTION_ID);
}

TEST_F(TestInstructionSet, TestGetInstructionId_TooBigInstruction) {
    uint16_t instructionId = GetInstructionId((27 << 8));
    EXPECT_EQ(instructionId, INVALID_INSTRUCTION_ID);
}

