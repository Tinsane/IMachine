//
// Created by vladimirlesk on 11.12.18.
//

#ifndef IMACHINE_SUBPROCESS_H
#define IMACHINE_SUBPROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "instructions.h"

struct Subprocess {
    uint32_t TicksToExecute;
    uint32_t TicksExecuted;
    uint16_t *Memory;
    uint16_t RegisterSets[REGISTER_CNT];
    bool Exited;
    uint8_t ExitCode;
};

extern struct Subprocess *
NewSubprocess(uint16_t *memory, uint32_t ticksToExecute, uint16_t stackAddr);

extern void DeleteSubprocess(struct Subprocess *subprocess);

// RunInstruction returns false only in case, when Subprocess has already exited.
extern bool RunInstruction(struct Subprocess *subprocess);

#endif //IMACHINE_SUBPROCESS_H
