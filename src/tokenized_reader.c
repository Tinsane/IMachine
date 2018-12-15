//
// Created by vladimirlesk on 15.12.18.
//

#include <malloc.h>
#include <ctype.h>
#include "tokenized_reader.h"

struct TokenizedReader TokenizedReaderFromBuffer(char *buffer) {
    struct TokenizedReader reader;
    reader.BufferPtr = buffer;
    return reader;
}

static void DropUntil(struct TokenizedReader *reader, bool (*stopCondition)(char)) {
    while (*reader->BufferPtr != 0) {
        if (stopCondition(*reader->BufferPtr)) {
            return;
        }
        ++reader->BufferPtr;
    }
}

static bool BFR_is_not_space(char c) {
    return !isspace(c);
}

void DropSpace(struct TokenizedReader *reader) {
    DropUntil(reader, BFR_is_not_space);
}

void ReadToken(struct TokenizedReader *reader, char *dst, size_t lengthLimit) {
    DropSpace(reader);
    while (*reader->BufferPtr != 0 && lengthLimit > 0) {
        if (isspace(*reader->BufferPtr)) {
            break;
        }
        *(dst++) = *reader->BufferPtr;
        ++reader->BufferPtr;
        --lengthLimit;
    }
    *dst = 0;
}
