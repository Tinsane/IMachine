//
// Created by vladimirlesk on 14.12.18.
//

#include <stddef.h>
#include <memory.h>
#include <ctype.h>
#include "assembler.h"
#include "macro.h"
#include "instructions.h"
#include "tokenized_reader.h"

void InitAssembler(struct Assembler *assembler) {
    size_t i;

    assembler->CurrentLineId = 0;
    for (i = 0; i < MAX_JUMP; ++i) {
        assembler->LabelsUsed[i][0] = 0;
        assembler->LabelsDeclared[i][0] = 0;
    }
}

static bool SetLineLabel(struct Assembler *assembler, size_t lineId, int8_t offset) {
    uint16_t mask = (*(uint8_t *) (&offset)) << (16u - BIG_OPCODE_SIZE - IMM8_SIZE);
    size_t changedLineId = lineId % MAX_JUMP;

    if (!(assembler->InstructionWindow[changedLineId] & LABEL_SUBSTITUTION_FLAG)) {
        return false;
    }
    assembler->InstructionWindow[changedLineId] &= (~LABEL_SUBSTITUTION_FLAG);
    assembler->InstructionWindow[changedLineId] |= mask;
    return true;
}

static bool SetOthersLabels(struct Assembler *assembler, char *label) {
    int offset;
    int lineId;

    if (label[0] == 0) {
        return true;
    }
    for (offset = -MAX_JUMP + 1; offset <= 0; ++offset) {
        lineId = (int) assembler->CurrentLineId + offset;
        if (lineId < 0) {
            continue;
        }
        if (strcmp(label, assembler->LabelsUsed[lineId % MAX_JUMP]) == 0) {
            RETURNF_IF(!SetLineLabel(assembler, (size_t) lineId, (int8_t) -offset), false,
                       "Redeclared a label for line %d.\n", lineId);
        }
    }
    return true;
}

static bool SetMyLabel(struct Assembler *assembler, char *innerLabel) {
    int offset;
    int lineId;

    if (innerLabel[0] == 0) {
        return true;
    }
    for (offset = -MAX_JUMP; offset < 0; ++offset) {
        lineId = (int) assembler->CurrentLineId + offset;
        if (lineId < 0) {
            continue;
        }
        if (strcmp(innerLabel, assembler->LabelsDeclared[lineId % MAX_JUMP]) == 0) {
            RETURNF_IF(!SetLineLabel(assembler, assembler->CurrentLineId, (int8_t) offset), false,
                       "Redeclared a label from line %d.\n", lineId);
        }
    }
    return true;
}

static bool DumpLine(struct Assembler *assembler, FILE *destination) {
    size_t instructionsWritten;
    size_t windowLineId = assembler->CurrentLineId % MAX_JUMP;
    if (assembler->CurrentLineId < MAX_JUMP) {
        return true;
    }
    if (assembler->LabelsUsed[windowLineId][0] != 0) {
        RETURNF_IF(assembler->InstructionWindow[windowLineId] & LABEL_SUBSTITUTION_FLAG, false,
                   "Couldn't find label substitution for line: %zu.\n",
                   assembler->CurrentLineId - MAX_JUMP);
    }
    instructionsWritten = fwrite(assembler->InstructionWindow + windowLineId,
                          2, 1, destination);
    RETURNF_IF(instructionsWritten != 1, false, "Failed to write instruction at line: %zu.\n",
               assembler->CurrentLineId - MAX_JUMP);
    return true;
}

#define MAX_LINE_LENGTH 60u

bool AssembleFile(struct BufferedFileReader *source, FILE *destination) {
    struct Assembler assembler;
    struct CodeLine line;
    bool parsingSucceeded;
    size_t i;
    char lineBuffer[MAX_LINE_LENGTH + 1u];
    struct TokenizedReader lineReader;

    InitAssembler(&assembler);
    while (true) {
        if (!ReadLine(source, lineBuffer, MAX_LINE_LENGTH)) {
            ERRORF("Failed to read line %zu.\n", assembler.CurrentLineId);
            return false;
        }
        if (lineBuffer[0] == 0 && IsEOF(source)) {
            break;
        }
        lineReader = TokenizedReaderFromBuffer(lineBuffer);
        ResetLine(&line);
        parsingSucceeded = ParseLine(&lineReader, &line);
        RETURNF_IF(!parsingSucceeded, false, "Failed to parse line %zu.\n",
                   assembler.CurrentLineId);
        if (line.InnerLabel[0] != 0) {
            line.RawInstruction |= LABEL_SUBSTITUTION_FLAG;
        }
        RETURNF_IF(!DumpLine(&assembler, destination), false, "");
        assembler.InstructionWindow[assembler.CurrentLineId] = line.RawInstruction;
        memcpy(assembler.LabelsUsed[assembler.CurrentLineId], line.InnerLabel, LABEL_MAX_LEN);
        RETURNF_IF(!SetMyLabel(&assembler, line.InnerLabel), false,
                   "Failed to get label for line %zu.\n", assembler.CurrentLineId);
        memcpy(assembler.LabelsDeclared[assembler.CurrentLineId], line.LineLabel, LABEL_MAX_LEN);
        RETURNF_IF(!SetOthersLabels(&assembler, line.LineLabel), false,
                   "Failed to distribute label of line %zu.\n", assembler.CurrentLineId);
        ++assembler.CurrentLineId;
    }
    for (i = 0; i < MAX_JUMP; ++i) {
        RETURNF_IF(!DumpLine(&assembler, destination), false, "");
        ++assembler.CurrentLineId;
    }
    return true;
}

#undef MAX_LINE_LENGTH
