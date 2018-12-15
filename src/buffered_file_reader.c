//
// Created by vladimirlesk on 11.12.18.
//

#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <malloc.h>
#include <ctype.h>
#include "buffered_file_reader.h"
#include "macro.h"

#define BUFFERED_DATA_START (reader->BufferedData + reader->BufferConsumed)
#define BUFFERED_DATA_SIZE (reader->BufferedDataSize - reader->BufferConsumed)

struct BufferedFileReader *NewBufferedFileReader(char *filename, size_t bufferSize) {
    FILE *file;
    struct BufferedFileReader *reader;

    file = fopen(filename, "r");
    RETURNF_IF(file == NULL, NULL,
               "Failed to open file: '%s', because of error: %d\n", filename, errno);
    reader = malloc(sizeof(struct BufferedFileReader) + bufferSize + 1);
    if (reader == NULL) {
        ERRORF("Failed to allocate buffered reader for file: '%s'\n", filename);
        RETURNF_IF(fclose(file) != 0, NULL, "Failed to close opened file: '%s'\n", filename);
        return NULL;
    }
    reader->ReadFrom = file;
    reader->BufferSize = bufferSize;
    reader->BufferedDataSize = 0;
    reader->BufferConsumed = 0;
    return reader;
}

void DeleteBufferedFileReader(struct BufferedFileReader *reader) {
    if (fclose(reader->ReadFrom) != 0) {
        ERROR("Failed to close read file!");
    }
    free(reader);
}

// ReadToBuffer returns true in case it succeeded to read at least one byte
static bool ReadToBuffer(struct BufferedFileReader *reader) {
    size_t bytesRead;
    if (feof(reader->ReadFrom) || ferror(reader->ReadFrom)) {
        return false;
    }
    if (reader->BufferConsumed != reader->BufferedDataSize) {
        memmove(reader->BufferedData, BUFFERED_DATA_START, BUFFERED_DATA_SIZE);
    }
    reader->BufferedDataSize -= reader->BufferConsumed;
    reader->BufferConsumed = 0;
    bytesRead = fread(BUFFERED_DATA_START, 1,
                      reader->BufferSize - reader->BufferedDataSize,
                      reader->ReadFrom);
    if (ferror(reader->ReadFrom)) {
        errno;
    }
    reader->BufferedDataSize += bytesRead;
    reader->BufferedData[reader->BufferSize] = 0;
    return bytesRead != 0;
}

bool ReadUint16(struct BufferedFileReader *reader, uint16_t *dst) {
    if (BUFFERED_DATA_SIZE < 2) {
        if (feof(reader->ReadFrom) || ferror(reader->ReadFrom)) {
            return false;
        }
        if (!ReadToBuffer(reader)) {
            return false;
        }
        if (BUFFERED_DATA_SIZE < 2) {
            return false;
        }
    }
    *dst = *(uint16_t *) (BUFFERED_DATA_START);
    reader->BufferConsumed += 2;
    return true;
}

static bool IsLineEnd(char c) {
    return c == '\n' || c == '\r';
}

static bool DropLineEnd(struct BufferedFileReader* reader) {
    while(true) {
        if (BUFFERED_DATA_SIZE == 0 && !ReadToBuffer(reader)) {
            return !IsError(reader);
        }
        if (!IsLineEnd(*BUFFERED_DATA_START)) {
            return true;
        }
        ++reader->BufferConsumed;
    }
}

bool ReadLine(struct BufferedFileReader* reader, char* dst, size_t lengthLimit) {
    while(true) {
        if (BUFFERED_DATA_SIZE == 0 && !ReadToBuffer(reader)) {
            if (IsError(reader)) {
                return false;
            } else {
                *dst = 0;
                return true;
            }
        }
        RETURNF_IF(lengthLimit == 0, false, "Can't fit line to buffer");
        if (IsLineEnd(*BUFFERED_DATA_START)) {
            *dst = 0;
            return DropLineEnd(reader);
        }
        *dst = *BUFFERED_DATA_START;
        ++dst;
        ++reader->BufferConsumed;
        --lengthLimit;
    }
}

extern bool IsError(struct BufferedFileReader *reader) {
    return ferror(reader->ReadFrom) && BUFFERED_DATA_SIZE == 0;
}

bool IsEOF(struct BufferedFileReader *reader) {
    return feof(reader->ReadFrom) && BUFFERED_DATA_SIZE == 0;
}

#undef BUFFERED_DATA_SIZE
#undef BUFFERED_DATA_START
