//
// Created by vladimirlesk on 08.12.18.
//

#ifndef IMACHINE_ASSEMBLER_H
#define IMACHINE_ASSEMBLER_H

#include <stddef.h>
#include <stdbool.h>
#include "buffered_file_reader.h"
#include "code_line.h"

#define MAX_JUMP 128u
#define LABEL_SUBSTITUTION_FLAG 1u

struct Assembler {
    char LabelsUsed[MAX_JUMP][LABEL_MAX_LEN + 1u];
    char LabelsDeclared[MAX_JUMP][LABEL_MAX_LEN + 1u];
    uint16_t InstructionWindow[MAX_JUMP];
    size_t CurrentLineId;
};

extern void InitAssembler(struct Assembler *assembler);

extern bool AssembleFile(struct BufferedFileReader *source, FILE *destination);

#endif //IMACHINE_ASSEMBLER_H
