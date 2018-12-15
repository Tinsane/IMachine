//
// Created by vladimirlesk on 08.12.18.
//

#include "instruction_set.h"
#include "macro.h"

uint8_t InstructionSet_ExitCode;

#define STACK_OVERFLOW_MSG "Stack overflow!"
#define UNALIGNED_STACK_ACCESS_MSG "Unaligned stack access!"
#define UNALIGNED_MEMORY_ACCESS_MSG "Unaligned memory access!"
#define DIVISION_BY_ZERO_MSG "Division by zero!"
#define IP_OUT_OF_MEMORY_MSG "Instruction pointer tried to leave memory!"

#define ACCESS_MEMORY(address) \
    memory[(address) >> 1u]

#define IP registers[IP_ID]
#define SP registers[SP_ID]
#define RX_MASK 7u
#define HIGH_FLAG 8u
#define RS_PTR(regId) \
    (((uint8_t*) (&registers[(regId) & RX_MASK])) + \
    (((regId) & HIGH_FLAG) ? 1 : 0))

#define DECLARE_INSTRUCTION(name) \
static bool Execute_ ##name(uint16_t rawOperation, uint16_t registers[], uint16_t memory[])

#define EXTRACT_SZ_BITS(mask, offset, result, sz) \
    do { \
        (offset) -= (sz); \
        (result) = (uint8_t) (((mask) >> (offset)) & ((1u << (sz)) - 1u)); \
    } while(0)

#define EXTRACT_ONE_ARG(mask, offsetSz, argName, argSz) \
    uint8_t offset = (offsetSz); \
    uint8_t (argName); \
    EXTRACT_SZ_BITS((mask), offset, (argName), (argSz))

#define EXTRACT_TWO_ARGS(mask, offsetSz, argName1, argSz1, argName2, argSz2) \
    uint8_t offset = (offsetSz); \
    uint8_t (argName1); \
    uint8_t (argName2); \
    EXTRACT_SZ_BITS((mask), offset, (argName1), (argSz1)); \
    EXTRACT_SZ_BITS((mask), offset, (argName2), (argSz2))


#define EXTRACT_THREE_ARGS(mask, offsetSz, argName1, argSz1, argName2, argSz2, argName3, argSz3) \
    uint8_t offset = (offsetSz); \
    uint8_t (argName1); \
    uint8_t (argName2); \
    uint8_t (argName3); \
    EXTRACT_SZ_BITS((mask), offset, (argName1), (argSz1)); \
    EXTRACT_SZ_BITS((mask), offset, (argName2), (argSz2)); \
    EXTRACT_SZ_BITS((mask), offset, (argName3), (argSz3))

#define FAIL_INSTRUCTION_IF(condition, ERROR_MSG) \
    do { \
        if (condition) { \
            ERROR(ERROR_MSG); \
            InstructionSet_ExitCode = 1; \
            return false; \
        } \
    } while(0)

#define CHECK_ADDRESS_ALIGNMENT(addr) FAIL_INSTRUCTION_IF((addr) & 1u, UNALIGNED_MEMORY_ACCESS_MSG)

#define CHECK_SP_ALIGNMENT() FAIL_INSTRUCTION_IF(SP & 1u, UNALIGNED_STACK_ACCESS_MSG)

#define SUBMIT_INSTRUCTION() \
    FAIL_INSTRUCTION_IF(IP + 2u == 0u, IP_OUT_OF_MEMORY_MSG); \
    IP += 2u; \
    return true

#define INCREASE_SP() \
    FAIL_INSTRUCTION_IF(SP + 1u == 0u || SP + 2u == 0u, STACK_OVERFLOW_MSG); \
    SP += 2u

#define DECREASE_SP() \
    FAIL_INSTRUCTION_IF(SP < 2u, STACK_OVERFLOW_MSG); \
    SP -= 2u

static bool AdjustIP(int8_t jumpDelta, uint16_t registers[]) {
    int32_t IPValue = jumpDelta;
    IPValue *= 2;
    IPValue += IP;
    FAIL_INSTRUCTION_IF(IPValue < 0 || IPValue > MEMORY_SZ, IP_OUT_OF_MEMORY_MSG);
    IP = (uint16_t) IPValue;
    return true;
}

DECLARE_INSTRUCTION(movv) {
    EXTRACT_TWO_ARGS(rawOperation, BIG_OPCODE_OFFSET, dstId, RS_ID_SZ, value, IMM8_SIZE);
    *RS_PTR(dstId) = value;
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(jne) {
    EXTRACT_TWO_ARGS(rawOperation, BIG_OPCODE_OFFSET, jumpDelta, IMM8_SIZE, registerId, RX_ID_SZ);
    if (registers[registerId] != 0) {
        return AdjustIP(jumpDelta, registers);
    }
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(je) {
    EXTRACT_TWO_ARGS(rawOperation, BIG_OPCODE_OFFSET, jumpDelta, IMM8_SIZE, registerId, RX_ID_SZ);
    if (registers[registerId] == 0) {
        return AdjustIP(jumpDelta, registers);
    }
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(cmpxchg) {
    EXTRACT_THREE_ARGS(rawOperation, BIG_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ, c, RX_ID_SZ);
    CHECK_ADDRESS_ALIGNMENT(registers[a]);
    if (ACCESS_MEMORY(registers[a]) == registers[b]) {
        ACCESS_MEMORY(registers[a]) = registers[c];
        registers[c] = registers[b];
    }
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(jmp) {
    EXTRACT_ONE_ARG(rawOperation, BIG_OPCODE_OFFSET, jumpDelta, IMM8_SIZE);
    return AdjustIP(jumpDelta, registers);
}

DECLARE_INSTRUCTION(call) {
    EXTRACT_ONE_ARG(rawOperation, SMALL_OPCODE_OFFSET, jumpDelta, IMM8_SIZE);
    CHECK_SP_ALIGNMENT();
    DECREASE_SP();
    FAIL_INSTRUCTION_IF(IP + 2u == 0u, IP_OUT_OF_MEMORY_MSG);
    ACCESS_MEMORY(SP) = (uint16_t) (IP + 2u);
    return AdjustIP(jumpDelta, registers);
}

DECLARE_INSTRUCTION(shr) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, registerId, RX_ID_SZ, shift, IMM4_SIZE);
    registers[registerId] >>= shift;
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(shl) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, registerId, RX_ID_SZ, shift, IMM4_SIZE);
    registers[registerId] <<= shift;
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(xor) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ);
    registers[a] ^= registers[b];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(sub) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ);
    registers[a] -= registers[b];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(or) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ);
    registers[a] |= registers[b];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(mul) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ);
    registers[a] *= registers[b];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(movr) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, dstId, RX_ID_SZ, srcId, RX_ID_SZ);
    registers[dstId] = registers[srcId];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(mov2) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, dstId, RX_ID_SZ, srcAddressId, RX_ID_SZ);
    CHECK_ADDRESS_ALIGNMENT(registers[srcAddressId]);
    registers[dstId] = ACCESS_MEMORY(registers[srcAddressId]);
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(mov1) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, dstAddressId, RX_ID_SZ, srcId, RX_ID_SZ);
    CHECK_ADDRESS_ALIGNMENT(registers[dstAddressId]);
    ACCESS_MEMORY(registers[dstAddressId]) = registers[srcId];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(div) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ);
    FAIL_INSTRUCTION_IF(registers[b] == 0, DIVISION_BY_ZERO_MSG);
    registers[a] /= registers[b];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(and) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ);
    registers[a] &= registers[b];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(add) {
    EXTRACT_TWO_ARGS(rawOperation, SMALL_OPCODE_OFFSET, a, RX_ID_SZ, b, RX_ID_SZ);
    registers[a] += registers[b];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(out) {
    EXTRACT_ONE_ARG(rawOperation, SMALL_OPCODE_OFFSET, regId, RS_ID_SZ);
    printf("%c", *RS_PTR(regId));
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(in) {
    EXTRACT_ONE_ARG(rawOperation, SMALL_OPCODE_OFFSET, regId, RS_ID_SZ);
    scanf("%c", RS_PTR(regId));
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(push) {
    EXTRACT_ONE_ARG(rawOperation, SMALL_OPCODE_OFFSET, registerId, RX_ID_SZ);
    CHECK_SP_ALIGNMENT();
    DECREASE_SP();
    ACCESS_MEMORY(SP) = registers[registerId];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(pop) {
    EXTRACT_ONE_ARG(rawOperation, SMALL_OPCODE_OFFSET, registerId, RX_ID_SZ);
    CHECK_SP_ALIGNMENT();
    registers[registerId] = ACCESS_MEMORY(SP);
    INCREASE_SP();
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(not) {
    EXTRACT_ONE_ARG(rawOperation, SMALL_OPCODE_OFFSET, registerId, RX_ID_SZ);
    registers[registerId] = ~registers[registerId];
    SUBMIT_INSTRUCTION();
}

DECLARE_INSTRUCTION(ret) {
    CHECK_SP_ALIGNMENT();
    IP = ACCESS_MEMORY(SP);
    INCREASE_SP();
    return true;
}

DECLARE_INSTRUCTION(reset) {
    InstructionSet_ExitCode = *RS_PTR(0u);
    return false;
}

DECLARE_INSTRUCTION(nop) {
    SUBMIT_INSTRUCTION();
}

bool (*INSTRUCTIONS[])(uint16_t, uint16_t[], uint16_t[]) = {
        Execute_movv,
        Execute_jne,
        Execute_je,
        Execute_cmpxchg,
        Execute_jmp,
        Execute_call,
        Execute_shr,
        Execute_shl,
        Execute_xor,
        Execute_sub,
        Execute_or,
        Execute_mul,
        Execute_movr,
        Execute_mov2,
        Execute_mov1,
        Execute_div,
        Execute_and,
        Execute_add,
        Execute_out,
        Execute_in,
        Execute_push,
        Execute_pop,
        Execute_not,
        Execute_ret,
        Execute_reset,
        Execute_nop
};

uint16_t GetInstructionId(uint16_t rawOperation) {
    uint16_t instructionId;
    if (rawOperation & BIG_INSTRUCTION_MARKER) {
        instructionId = (uint16_t) ((rawOperation ^ BIG_INSTRUCTION_MARKER) >> BIG_OPCODE_OFFSET);
        if (instructionId >= BIG_INSTRUCTION_CNT) {
            return INVALID_INSTRUCTION_ID;
        }
        return instructionId;
    }
    instructionId = rawOperation >> SMALL_OPCODE_OFFSET;
    if (instructionId < BIG_INSTRUCTION_CNT || instructionId >= INSTRUCTION_CNT) {
        return INVALID_INSTRUCTION_ID;
    }
    return instructionId;
}

#undef DECREASE_SP
#undef INCREASE_SP
#undef SUBMIT_INSTRUCTION
#undef CHECK_SP_ALIGNMENT
#undef CHECK_ADDRESS_ALIGNMENT
#undef FAIL_INSTRUCTION_IF
#undef EXTRACT_THREE_ARGS
#undef EXTRACT_TWO_ARGS
#undef EXTRACT_ONE_ARG
#undef EXTRACT_SZ_BITS
#undef DECLARE_INSTRUCTION
#undef RS_PTR
#undef SP
#undef IP
#undef ACCESS_MEMORY
#undef IP_OUT_OF_MEMORY_MSG
#undef DIVISION_BY_ZERO_MSG
#undef UNALIGNED_MEMORY_ACCESS_MSG
#undef UNALIGNED_STACK_ACCESS_MSG
#undef STACK_OVERFLOW_MSG
