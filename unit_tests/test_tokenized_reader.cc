extern "C" {
#include "../src/tokenized_reader.h"
}

#include <string>
#include "gtest/gtest.h"

using std::string;

class TestTokenizedReader : public ::testing::Test {
};

TEST_F(TestTokenizedReader, TestDropSpace) {
    char buffer[] = " \txyz";
    auto reader = TokenizedReaderFromBuffer(buffer);
    DropSpace(&reader);
    EXPECT_EQ(reader.BufferPtr, buffer + 2);
}

void TestReadToken(char buffer[], string result) {
    auto reader = TokenizedReaderFromBuffer(buffer);
    char dst[6];
    ReadToken(&reader, dst, 5);
    EXPECT_STREQ(dst, result.c_str());
}

TEST_F(TestTokenizedReader, TestReadToken_UntilBufferEnd) {
    char buffer[] = "abc";
    TestReadToken(buffer, "abc");
}

TEST_F(TestTokenizedReader, TestReadToken_UntilSpace) {
    char buffer[] = "abc  ";
    TestReadToken(buffer, "abc");
}

TEST_F(TestTokenizedReader, TestReadToken_SkipSpaces) {
    char buffer[] = " \t abc";
    TestReadToken(buffer, "abc");
}

TEST_F(TestTokenizedReader, TestReadToken_Truncate) {
    char buffer[] = "abcdefgh";
    TestReadToken(buffer, "abcde");
}

