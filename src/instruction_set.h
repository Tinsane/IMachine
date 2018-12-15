//
// Created by vladimirlesk on 08.12.18.
//

#ifndef IMACHINE_INSTRUCTION_SET_H
#define IMACHINE_INSTRUCTION_SET_H

#include <stdbool.h>
#include <stdint.h>
#include "instructions.h"

#define SP_ID 7u
#define IP_ID 8u
#define HIGH_FLAG 8u
#define RX_MASK 7u

extern bool
(*INSTRUCTIONS[INSTRUCTION_CNT])(uint16_t rawOperation, uint16_t registers[], uint16_t memory[]);

extern uint16_t GetInstructionId(uint16_t rawOperation);

extern uint8_t InstructionSet_ExitCode;

#endif //IMACHINE_INSTRUCTION_SET_H
