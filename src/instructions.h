//
// Created by vladimirlesk on 08.12.18.
//

#ifndef IMACHINE_INSTRUCTIONS_H
#define IMACHINE_INSTRUCTIONS_H

#define BIG_INSTRUCTION_CNT 5u
#define SMALL_INSTRUCTION_CNT 21u
#define INSTRUCTION_CNT 26u
#define BIG_INSTRUCTION_MARKER (1u << 15u)
#define MEMORY_SZ (1u << 16u)
#define RX_ID_SZ 3u
#define RS_ID_SZ 4u
#define IMM4_SIZE 4u
#define IMM8_SIZE 8u
#define SMALL_OPCODE_SIZE 8u
#define BIG_OPCODE_SIZE 4u
#define RAW_OP_SIZE 16u
#define INVALID_INSTRUCTION_ID (1u << 8u)
#define BIG_OPCODE_OFFSET (RAW_OP_SIZE - BIG_OPCODE_SIZE)
#define SMALL_OPCODE_OFFSET (RAW_OP_SIZE - SMALL_OPCODE_SIZE)

#define REGISTER_CNT 9u

#endif //IMACHINE_INSTRUCTIONS_H
