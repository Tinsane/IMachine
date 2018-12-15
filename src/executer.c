//
// Created by vladimirlesk on 11.12.18.
//

#include <malloc.h>
#include <memory.h>
#include "executer.h"
#include "macro.h"
#include "buffered_file_reader.h"

struct Executer *NewExecuter(uint16_t subprocessCnt, uint32_t ticksPerProcessIteration) {
    struct Executer *executer;
    uint32_t executerSz;
    uint16_t i;

    RETURNF_IF(subprocessCnt == 0, NULL, "Expected non zero number of subprocess!");
    RETURNF_IF(ticksPerProcessIteration == 0, NULL,
               "Expected non zero number of ticks per process iteration!");
    executerSz = sizeof(struct Executer) + subprocessCnt * sizeof(struct Subprocess *);
    executer = malloc(executerSz);
    RETURNF_IF(executer == NULL, NULL, "Failed to allocate an executer!");
    executer->SubprocessCnt = subprocessCnt;
    executer->TicksPerProcessIteration = ticksPerProcessIteration;
    memset(executer->Memory, 0, sizeof(uint16_t) * MEMORY_SIZE);
    for (i = 0; i < subprocessCnt; ++i) {
        executer->Subprocesses[i] = NULL;
    }
    return executer;
}

void DeleteExecuter(struct Executer *executer) {
    uint16_t i;
    for (i = 0; i < executer->SubprocessCnt; ++i) {
        DeleteSubprocess(executer->Subprocesses[i]);
    }
    free(executer);
}

bool LoadSubprocess(struct Executer *executer, uint16_t subprocessId, uint16_t codeOffset,
                    struct BufferedFileReader *codeProducer, uint32_t ticksToExecute) {
    uint16_t i = codeOffset >> 1u;
    uint16_t opCode;
    RETURNF_IF(subprocessId >= executer->SubprocessCnt, false, "Unexpected subprocess id: %d!",
               subprocessId);
    RETURNF_IF(executer->Subprocesses[subprocessId] != NULL, false,
               "Subprocess under id %d is already initialized!", subprocessId);
    RETURNF_IF(codeOffset & 1u, false, "Can't put code to odd offset for subprocess: %d!",
               subprocessId);
    while (true) {
        if (!ReadUint16(codeProducer, &opCode)) {
            RETURNF_IF(IsError(codeProducer), false,
                       "Got error while loading subprocess %d from file!", subprocessId);
            // No error and no EOF, just means that we have not enough data for one instruction.
            // So, we have an odd number of bytes in code file, which is invalid
            RETURNF_IF(!IsEOF(codeProducer), false,
                       "Got an error while reading subprocess codes, last code is not full!");
            break;
        }
        RETURNF_IF(executer->Memory[i] != 0, false, "Can't load intersecting subprocesses!");
        executer->Memory[i] = opCode;
        ++i;
        RETURNF_IF(i == 0, false, "Can't load code of subprocess %d in memory with given offset!",
                   subprocessId);
    }
    executer->Subprocesses[subprocessId] = NewSubprocess(executer->Memory, ticksToExecute,
                                                         codeOffset);
    RETURNF_IF(executer->Subprocesses[subprocessId] == NULL, false,
               "Failed to allocate a subprocess %d!", subprocessId);
    return true;
}

#define CURRENT_SUBPROCESS executer->Subprocesses[executer->CurrentSubprocessId]

static bool FindContinuingSubprocess(struct Executer *executer) {
    uint16_t i;
    for (i = 0; i < executer->SubprocessCnt; ++i) {
        ++executer->CurrentSubprocessId;
        if (executer->CurrentSubprocessId == executer->SubprocessCnt) {
            executer->CurrentSubprocessId = 0;
        }
        if (!CURRENT_SUBPROCESS->Exited) {
            break;
        }
    }
    return !CURRENT_SUBPROCESS->Exited;
}

extern bool RunExecution(struct Executer *executer) {
    uint32_t i;
    for (i = 0; i < executer->SubprocessCnt; ++i) {
        RETURNF_IF(executer->Subprocesses[i] == NULL, false,
                   "Can't start execution before loading all the processes!");
    }
    executer->CurrentSubprocessId = 0;
    while (true) {
        if (!FindContinuingSubprocess(executer)) {
            break;
        }
        for (i = 0; i < executer->TicksPerProcessIteration; ++i) {
            RunInstruction(CURRENT_SUBPROCESS);
            if (CURRENT_SUBPROCESS->Exited) {
                ERRORF("Subprocess %d exited with code: %d\n",
                       executer->CurrentSubprocessId, CURRENT_SUBPROCESS->ExitCode);
                break;
            }
        }
    }
    return true;
}
