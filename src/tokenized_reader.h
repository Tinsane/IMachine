//
// Created by vladimirlesk on 15.12.18.
//

#ifndef IMACHINE_TOKENIZED_READER_H
#define IMACHINE_TOKENIZED_READER_H

#include <stddef.h>
#include <stdbool.h>

struct TokenizedReader {
    char const *BufferPtr;
};

extern struct TokenizedReader TokenizedReaderFromBuffer(char const *buffer);

extern void ReadToken(struct TokenizedReader *reader, char *dst, size_t lengthLimit);

extern void DropSpace(struct TokenizedReader *reader);

#endif //IMACHINE_TOKENIZED_READER_H
