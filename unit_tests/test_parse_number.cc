extern "C" {
#include "../src/parse_number.h"
}

#include "gtest/gtest.h"

class TestParseNumber : public ::testing::Test {
};

void TestParseUInt4_Success(char str[], uint16_t expected) {
    uint16_t result;
    EXPECT_TRUE(ParseUInt4(str, &result));
    EXPECT_EQ(result, expected);
}

TEST_F(TestParseNumber, TestParseUInt4_Sanity) {
    char buffer[] = "12";
    TestParseUInt4_Success(buffer, 12);
}

TEST_F(TestParseNumber, TestParseUInt4_Zero) {
    char buffer[] = "0";
    TestParseUInt4_Success(buffer, 0);
}

TEST_F(TestParseNumber, TestParseUInt4_Max) {
    char buffer[] = "15";
    TestParseUInt4_Success(buffer, 15);
}

TEST_F(TestParseNumber, TestParseUInt4_Overflow) {
    char buffer[] = "16";
    EXPECT_FALSE(ParseUInt4(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt4_NonDigitsBegin) {
    char buffer[] = "abc12";
    EXPECT_FALSE(ParseUInt4(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt4_NonDigitsEnd) {
    char buffer[] = "3abc";
    EXPECT_FALSE(ParseUInt4(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt4_Empty) {
    char buffer[] = "";
    EXPECT_FALSE(ParseUInt4(buffer, NULL));
}

void TestParseUInt8_Success(char str[], uint16_t expected) {
    uint16_t result;
    EXPECT_TRUE(ParseUInt8(str, &result));
    EXPECT_EQ(result, expected);
}

TEST_F(TestParseNumber, TestParseUInt8_Sanity) {
    char buffer[] = "12";
    TestParseUInt8_Success(buffer, 12);
}

TEST_F(TestParseNumber, TestParseUInt8_Zero) {
    char buffer[] = "0";
    TestParseUInt8_Success(buffer, 0);
}

TEST_F(TestParseNumber, TestParseUInt8_Max) {
    char buffer[] = "255";
    TestParseUInt8_Success(buffer, 255);
}

TEST_F(TestParseNumber, TestParseUInt8_Overflow) {
    char buffer[] = "256";
    EXPECT_FALSE(ParseUInt8(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt8_NonDigitsBegin) {
    char buffer[] = "abc12";
    EXPECT_FALSE(ParseUInt8(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt8_NonDigitsEnd) {
    char buffer[] = "3abc";
    EXPECT_FALSE(ParseUInt8(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt8_Empty) {
    char buffer[] = "";
    EXPECT_FALSE(ParseUInt8(buffer, NULL));
}

void TestParseUInt16_Success(char str[], uint16_t expected) {
    uint16_t result;
    EXPECT_TRUE(ParseUInt16(str, &result));
    EXPECT_EQ(result, expected);
}

TEST_F(TestParseNumber, TestParseUInt16_Sanity) {
    char buffer[] = "12";
    TestParseUInt16_Success(buffer, 12);
}

TEST_F(TestParseNumber, TestParseUInt16_Zero) {
    char buffer[] = "0";
    TestParseUInt16_Success(buffer, 0);
}

TEST_F(TestParseNumber, TestParseUInt16_Max) {
    char buffer[] = "65535";
    TestParseUInt16_Success(buffer, (1 << 16) - 1);
}

TEST_F(TestParseNumber, TestParseUInt16_Overflow) {
    char buffer[] = "65536";
    EXPECT_FALSE(ParseUInt4(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt16_NonDigitsBegin) {
    char buffer[] = "abc12";
    EXPECT_FALSE(ParseUInt16(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt16_NonDigitsEnd) {
    char buffer[] = "3abc";
    EXPECT_FALSE(ParseUInt16(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt16_Empty) {
    char buffer[] = "";
    EXPECT_FALSE(ParseUInt16(buffer, NULL));
}

void TestParseUInt32_Success(char str[], uint32_t expected) {
    uint32_t result;
    EXPECT_TRUE(ParseUInt32(str, &result));
    EXPECT_EQ(result, expected);
}

TEST_F(TestParseNumber, TestParseUInt32_Sanity) {
    char buffer[] = "12";
    TestParseUInt32_Success(buffer, 12);
}

TEST_F(TestParseNumber, TestParseUInt32_Zero) {
    char buffer[] = "0";
    TestParseUInt32_Success(buffer, 0);
}

TEST_F(TestParseNumber, TestParseUInt32_Max) {
    char buffer[] = "4294967295";
    TestParseUInt32_Success(buffer, (1LL << 32) - 1);
}

TEST_F(TestParseNumber, TestParseUInt32_Overflow) {
    char buffer[] = "4294967296";
    EXPECT_FALSE(ParseUInt4(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt32_NonDigitsBegin) {
    char buffer[] = "abc12";
    EXPECT_FALSE(ParseUInt32(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt32_NonDigitsEnd) {
    char buffer[] = "3abc";
    EXPECT_FALSE(ParseUInt32(buffer, NULL));
}

TEST_F(TestParseNumber, TestParseUInt32_Empty) {
    char buffer[] = "";
    EXPECT_FALSE(ParseUInt32(buffer, NULL));
}

