//
// Created by vladimirlesk on 13.12.18.
//

#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <ctype.h>
#include "arg_parse.h"

#define PARSE_NUMBER(FunctionName, TNum, TBigger, TNumLowerBound, TNumUpperBound) \
bool FunctionName(char const *string, TNum *result) { \
    TBigger prefixValue = 0; \
    size_t string_len = strlen(string); \
    size_t i; \
    for (i = 0; i != string_len; ++i) { \
        if (!isdigit(string[i])) { \
            return false; \
        } \
        prefixValue = (TBigger) (prefixValue * 10 + (string[i] - '0')); \
        if (prefixValue > (TNumUpperBound)) { \
            return false; \
        } \
    } \
    if (prefixValue < (TNumLowerBound)) { \
        return false; \
    } \
    *result = (TNum) prefixValue; \
    return true; \
}

PARSE_NUMBER(ParseUInt4, uint16_t, uint16_t, 0, (1u << 4u) - 1u)

PARSE_NUMBER(ParseUInt8, uint16_t, uint16_t, 0, UINT8_MAX)

PARSE_NUMBER(ParseUInt16, uint16_t, uint32_t, 0, UINT16_MAX)

PARSE_NUMBER(ParseUInt32, uint32_t, uint64_t, 0, UINT32_MAX)

#undef PARSE_NUMBER
