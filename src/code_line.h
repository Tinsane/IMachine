//
// Created by vladimirlesk on 14.12.18.
//

#ifndef IMACHINE_CODE_LINE_H
#define IMACHINE_CODE_LINE_H

#include <stdbool.h>
#include <stdint.h>

#define LABEL_MAX_LEN 20u

struct CodeLine {
    char LineLabel[LABEL_MAX_LEN + 1];
    char InnerLabel[LABEL_MAX_LEN + 1];
    uint16_t RawInstruction;
};

struct TokenizedReader;

extern bool ParseLine(struct TokenizedReader *source, struct CodeLine *codeLine);

extern void ResetLine(struct CodeLine *line);

extern char *INSTRUCTION_NAMES;

#endif //IMACHINE_CODE_LINE_H
