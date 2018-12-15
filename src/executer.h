//
// Created by vladimirlesk on 11.12.18.
//

#ifndef IMACHINE_EXECUTER_H
#define IMACHINE_EXECUTER_H

#include <stdint.h>
#include <stdbool.h>
#include "instructions.h"
#include "subprocess.h"

#define MEMORY_SIZE (1u << 15u)

struct BufferedFileReader;

struct Executer {
    uint16_t Memory[MEMORY_SIZE];
    uint32_t TicksPerProcessIteration;
    uint16_t SubprocessCnt;
    uint16_t CurrentSubprocessId;
    struct Subprocess *Subprocesses[];
};

extern struct Executer *NewExecuter(uint16_t subprocessCnt, uint32_t ticksPerProcessIteration);

extern void DeleteExecuter(struct Executer *executer);

extern bool LoadSubprocess(struct Executer *executer, uint16_t subprocessId, uint16_t codeOffset,
                           struct BufferedFileReader* codeProducer, uint32_t ticksToExecute);

extern bool RunExecution(struct Executer *executer);

#endif //IMACHINE_EXECUTER_H
