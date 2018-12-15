//
// Created by vladimirlesk on 11.12.18.
//

#ifndef IMACHINE_BUFFERED_FILE_READER_H
#define IMACHINE_BUFFERED_FILE_READER_H

#include <stdbool.h>
#include <stdint.h>
#include <bits/types/FILE.h>

struct BufferedFileReader {
    FILE* ReadFrom;
    size_t BufferSize;
    size_t BufferedDataSize;
    size_t BufferConsumed;
    int ErrNo;
    char BufferedData[];
};

extern struct BufferedFileReader* NewBufferedFileReader(char *filename, size_t bufferSize);

extern void DeleteBufferedFileReader(struct BufferedFileReader *reader);

extern bool IsEOF(struct BufferedFileReader* reader);

extern bool IsError(struct BufferedFileReader* reader);

extern bool ReadLine(struct BufferedFileReader* reader, char* dst, size_t lengthLimit);

extern int GetErrNo(struct BufferedFileReader* reader);

// Returns false if can't read uint16_t
extern bool ReadUint16(struct BufferedFileReader* reader, uint16_t* dst);

#endif //IMACHINE_BUFFERED_FILE_READER_H
