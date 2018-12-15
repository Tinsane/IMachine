//
// Created by vladimirlesk on 11.12.18.
//

#include <malloc.h>
#include <memory.h>
#include "subprocess.h"
#include "instruction_set.h"
#include "macro.h"

#define EXIT_WITH_ERROR_IF(condition, errorMsg) \
    do { \
        if (condition) { \
            ERROR(errorMsg); \
            subprocess->ExitCode = 1; \
            subprocess->Exited = true; \
            return false; \
        } \
    } while(0)


struct Subprocess *NewSubprocess(uint16_t *memory, uint32_t ticksToExecute, uint16_t stackAddr) {
    struct Subprocess *subprocess = malloc(sizeof(struct Subprocess));
    if (subprocess == NULL) {
        return NULL;
    }
    memset(subprocess, 0, sizeof(struct Subprocess));
    subprocess->Memory = memory;
    subprocess->TicksToExecute = ticksToExecute;
    subprocess->RegisterSets[SP_ID] = stackAddr;
    subprocess->RegisterSets[IP_ID] = stackAddr;
    return subprocess;
}

void DeleteSubprocess(struct Subprocess *subprocess) {
    free(subprocess);
}

bool RunInstruction(struct Subprocess *subprocess) {
    uint16_t rawOperation;
    uint16_t IP = subprocess->RegisterSets[IP_ID];
    uint16_t instructionId;
    EXIT_WITH_ERROR_IF(IP & 1u, "Unaligned instruction access error!");
    rawOperation = subprocess->Memory[IP >> 1u];
    instructionId = GetInstructionId(rawOperation);
    EXIT_WITH_ERROR_IF(instructionId == INVALID_INSTRUCTION_ID,
                       "Tried to run an instruction with invalid id!");
    EXIT_WITH_ERROR_IF(subprocess->TicksToExecute != 0 &&
                       subprocess->TicksExecuted == subprocess->TicksToExecute,
                       "Subprocess exited because of timeout!");
    ++subprocess->TicksExecuted;
    if (!INSTRUCTIONS[instructionId](rawOperation, subprocess->RegisterSets, subprocess->Memory)) {
        subprocess->ExitCode = InstructionSet_ExitCode;
        subprocess->Exited = true;
        return false;
    }
    return true;
}

#undef EXIT_WITH_ERROR_IF
