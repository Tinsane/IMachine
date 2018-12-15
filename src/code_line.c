//
// Created by vladimirlesk on 14.12.18.
//

#include <stddef.h>
#include <memory.h>
#include <ctype.h>
#include "code_line.h"
#include "macro.h"
#include "instructions.h"
#include "parse_number.h"
#include "instruction_set.h"
#include "tokenized_reader.h"

struct ArgParser {
    bool (*ParseArg)(char const *argStr, uint16_t *value, char *label);

    size_t ArgSize;
};

#define DECLARE_PARSE_ARG(argType) \
static bool Parse_ ##argType(char const* argStr, uint16_t* value, char* label)

DECLARE_PARSE_ARG(IMM4) {
    if (argStr[0] != '#') {
        return false;
    }
    return ParseUInt4(argStr + 1, value);
}

DECLARE_PARSE_ARG(IMM8) {
    if (argStr[0] != '#') {
        return false;
    }
    return ParseUInt8(argStr + 1, value);
}

DECLARE_PARSE_ARG(RX) {
    if (argStr[0] != 'R' || !('0' <= argStr[1] && argStr[1] <= '7') || argStr[2] != 'X' ||
        argStr[3] != 0) {
        return false;
    }
    *value = (uint16_t) (argStr[1] - '0');
    return true;
}

DECLARE_PARSE_ARG(RX_REF) {
    if (argStr[0] != '(' || argStr[1] != 'R' || !('0' <= argStr[2] && argStr[2] <= '7') ||
        argStr[3] != 'X' || argStr[4] != ')' || argStr[5] != 0) {
        return false;
    }
    *value = (uint16_t) (argStr[2] - '0');
    return true;
}

DECLARE_PARSE_ARG(RS) {
    if (argStr[0] != 'R' || !('0' <= argStr[1] && argStr[1] <= '7') ||
        (argStr[2] != 'L' && argStr[2] != 'H') || argStr[3] != 0) {
        return false;
    }
    *value = (uint16_t) (argStr[1] - '0');
    if (argStr[2] == 'H') {
        *value |= HIGH_FLAG;
    }
    return true;
}

DECLARE_PARSE_ARG(Label) {
    size_t argStrLength;
    size_t i;
    argStrLength = strlen(argStr);
    if (argStrLength < 2 || argStrLength > LABEL_MAX_LEN) {
        return false;
    }
    for (i = 0; i < argStrLength - 1; ++i) {
        if (!isalnum(argStr[i])) {
            return false;
        }
    }
    memcpy(label, argStr, argStrLength + 1);
    return true;
}

#define MAKE_ARG_PARSER(type, size) \
static struct ArgParser ParserOf_ ##type = { \
        .ParseArg = Parse_ ##type, \
        .ArgSize = (size) \
}

MAKE_ARG_PARSER(IMM4, 4);
MAKE_ARG_PARSER(IMM8, 8);
MAKE_ARG_PARSER(RX, 3);
MAKE_ARG_PARSER(RX_REF, 3);
MAKE_ARG_PARSER(RS, 4);
MAKE_ARG_PARSER(Label, 8);

static bool ParseNArgs(struct TokenizedReader *reader, size_t n, struct ArgParser *parsers,
                       struct CodeLine *line) {
    size_t offset = RAW_OP_SIZE;
    size_t i;
    uint16_t argValue;
    bool argParsingResult;
    char argStr[LABEL_MAX_LEN + 1];

    for (i = 0; i < n; ++i) {
        ReadToken(reader, argStr, LABEL_MAX_LEN + 1);
        argParsingResult = parsers[i].ParseArg(argStr, &argValue, line->InnerLabel);
        RETURNF_IF(!argParsingResult, false, "Failed to parse argument: %zu.\n", i);
        offset -= parsers[i].ArgSize;
        line->RawInstruction |= argValue << offset;
    }

    if (line->InnerLabel[0] != 0) {
        line->RawInstruction &= ~1u;
    }
    return true;
}

#define MOVV_MASK 0u
#define MOV1_MASK 1u
#define MOV2_MASK 2u
#define MOVR_MASK 3u
#define MOV_MASK 3u

#define MOVV_ID 0u
#define MOV1_ID 14u
#define MOV2_ID 13u
#define MOVR_ID 12u

static bool ParseMovArgs(struct ArgParser *parsers, struct CodeLine *line,
                         char *argStr1, char *argStr2, uint16_t movMask) {
    size_t offset = RAW_OP_SIZE;
    uint16_t argValue1;
    uint16_t argValue2;
    bool argParsingResult;
    argParsingResult = parsers[0].ParseArg(argStr1, &argValue1, line->InnerLabel);
    if (!argParsingResult) {
        return false;
    }
    argParsingResult = parsers[1].ParseArg(argStr2, &argValue2, line->InnerLabel);
    if (!argParsingResult) {
        return false;
    }
    offset -= parsers[0].ArgSize;
    line->RawInstruction |= argValue1 << offset;
    offset -= parsers[1].ArgSize;
    line->RawInstruction |= argValue2 << offset;
    line->RawInstruction |= movMask;
    return true;
}

static bool Parse_mov(struct TokenizedReader *reader, struct CodeLine *line) {
    char argStr1[LABEL_MAX_LEN + 1];
    char argStr2[LABEL_MAX_LEN + 1];
    struct ArgParser movrParsers[] = {ParserOf_RX, ParserOf_RX};
    struct ArgParser mov1Parsers[] = {ParserOf_RX_REF, ParserOf_RX};
    struct ArgParser mov2Parsers[] = {ParserOf_RX, ParserOf_RX_REF};
    struct ArgParser movvParsers[] = {ParserOf_RS, ParserOf_IMM8};

    ReadToken(reader, argStr1, LABEL_MAX_LEN + 1);
    ReadToken(reader, argStr2, LABEL_MAX_LEN + 1);
    if (ParseMovArgs(movrParsers, line, argStr1, argStr2, MOVR_MASK)) {
        return true;
    }
    if (ParseMovArgs(mov1Parsers, line, argStr1, argStr2, MOV1_MASK)) {
        return true;
    }
    if (ParseMovArgs(mov2Parsers, line, argStr1, argStr2, MOV2_MASK)) {
        return true;
    }
    if (ParseMovArgs(movvParsers, line, argStr1, argStr2, MOVV_MASK)) {
        return true;
    }
    return false;
}

static bool Parse_jne(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_Label, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_je(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_Label, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_cmpxchg(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX_REF, ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 3, parserArray, line);
}

static bool Parse_jmp(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_Label};
    return ParseNArgs(reader, 1, parserArray, line);
}

static bool Parse_call(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_IMM8};
    return ParseNArgs(reader, 1, parserArray, line);
}

static bool Parse_shr(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_IMM4};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_shl(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_IMM4};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_xor(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_sub(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_or(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_mul(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_div(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_and(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_add(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX, ParserOf_RX};
    return ParseNArgs(reader, 2, parserArray, line);
}

static bool Parse_out(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RS};
    return ParseNArgs(reader, 1, parserArray, line);
}

static bool Parse_in(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RS};
    return ParseNArgs(reader, 1, parserArray, line);
}

static bool Parse_push(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX};
    return ParseNArgs(reader, 1, parserArray, line);
}

static bool Parse_pop(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX};
    return ParseNArgs(reader, 1, parserArray, line);
}

static bool Parse_not(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX};
    return ParseNArgs(reader, 1, parserArray, line);
}

static bool Parse_ret(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX};
    return ParseNArgs(reader, 0, parserArray, line);
}

static bool Parse_reset(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX};
    return ParseNArgs(reader, 0, parserArray, line);
}

static bool Parse_nop(struct TokenizedReader *reader, struct CodeLine *line) {
    struct ArgParser parserArray[] = {ParserOf_RX};
    return ParseNArgs(reader, 0, parserArray, line);
}

static bool (*INSTRUCTION_PARSERS[INSTRUCTION_CNT])(struct TokenizedReader *reader,
                                                    struct CodeLine *line) = {
        Parse_mov, // movv, 0
        Parse_jne,
        Parse_je,
        Parse_cmpxchg,
        Parse_jmp,
        Parse_call,
        Parse_shr,
        Parse_shl,
        Parse_xor,
        Parse_sub,
        Parse_or,
        Parse_mul,
        Parse_mov, // movr, 12
        Parse_mov, // mov2, 13
        Parse_mov, // mov1, 14
        Parse_div,
        Parse_and,
        Parse_add,
        Parse_out,
        Parse_in,
        Parse_push,
        Parse_pop,
        Parse_not,
        Parse_ret,
        Parse_reset,
        Parse_nop
};

static uint16_t ParseInstructionId(char *instructionToParse) {
    char *curInstruction = INSTRUCTION_NAMES;
    uint16_t instructionId;
    char *instructionToParsePtr;
    bool match;
    for (instructionId = 0; instructionId < INSTRUCTION_CNT; ++instructionId) {
        instructionToParsePtr = instructionToParse;
        match = true;
        while (!isspace(*curInstruction)) {
            match = match && *curInstruction == *instructionToParsePtr;
            ++curInstruction;
            ++instructionToParsePtr;
        }
        if (match && *instructionToParsePtr == 0) {
            return instructionId;
        }
        ++curInstruction;
    }
    return INVALID_INSTRUCTION_ID;
}

static bool ParseInstruction(struct TokenizedReader *source, struct CodeLine *codeLine,
                             uint16_t instructionId) {
    if (!INSTRUCTION_PARSERS[instructionId](source, codeLine)) {
        return false;
    }
    // corner case with mov
    if (instructionId == MOVV_ID) {
        switch (codeLine->RawInstruction & MOV_MASK) {
            case MOV1_MASK:
                instructionId = MOV1_ID;
                break;
            case MOV2_MASK:
                instructionId = MOV2_ID;
                break;
            case MOVR_MASK:
                instructionId = MOVR_ID;
                break;
            case MOVV_MASK:
                instructionId = MOVV_ID;
                break;
            default:
                break;
        }
    }
    if (instructionId < BIG_INSTRUCTION_CNT) {
        codeLine->RawInstruction >>= BIG_OPCODE_SIZE;
        codeLine->RawInstruction |= (instructionId) << BIG_OPCODE_OFFSET;
        codeLine->RawInstruction |= BIG_INSTRUCTION_MARKER;
    } else {
        codeLine->RawInstruction >>= SMALL_OPCODE_SIZE;
        codeLine->RawInstruction |= instructionId << SMALL_OPCODE_OFFSET;
    }
    return true;
}

bool ParseLine(struct TokenizedReader *source, struct CodeLine *codeLine) {
    char token[LABEL_MAX_LEN + 2];
    size_t tokenLength;
    uint16_t instructionId;

    ReadToken(source, token, LABEL_MAX_LEN + 1);
    tokenLength = strlen(token);
    RETURNF_IF(tokenLength == LABEL_MAX_LEN + 1, false, "Met unexpectedly big token.\n");
    if (token[tokenLength - 1] == ':') {
        RETURNF_IF(codeLine->LineLabel[0] != 0, false, "Expected one label in line.\n");
        memcpy(codeLine->LineLabel, token, tokenLength);
        codeLine->LineLabel[tokenLength - 1] = 0;
        return ParseLine(source, codeLine);
    }
    instructionId = ParseInstructionId(token);
    RETURNF_IF(instructionId == INVALID_INSTRUCTION_ID, false,
               "Expected an instruction, but got: '%s'.\n", token);
    RETURNF_IF(!ParseInstruction(source, codeLine, instructionId), false,
               "Failed to parse arguments.\n");
    return true;
}

void ResetLine(struct CodeLine *line) {
    line->LineLabel[0] = 0;
    line->InnerLabel[0] = 0;
    line->RawInstruction = 0;
}

char *INSTRUCTION_NAMES = "mov jne je cmpxchg jmp call shr shl xor sub or mul mov mov "
                          "mov div and add out in push pop not ret reset nop ";

#undef MOVR_ID
#undef MOV2_ID
#undef MOV1_ID
#undef MOVV_ID
#undef MOV_MASK
#undef MOVR_MASK
#undef MOV2_MASK
#undef MOV1_MASK
#undef MOVV_MASK

#undef MAKE_ARG_PARSER
#undef MAKE_ARG_PARSER
