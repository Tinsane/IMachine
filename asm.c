#include <stdio.h>
#include <errno.h>
#include "src/macro.h"
#include "src/buffered_file_reader.h"
#include "src/assembler.h"

int main(int argc, char** argv) {
    int returnValue = 0;
    struct BufferedFileReader* codeFileReader = NULL;
    FILE* operationFile = NULL;

#define FAILF_IF(condition, ...) \
    do { \
        if (condition) { \
            ERRORF(__VA_ARGS__); \
            returnValue = 1; \
            goto cleanup; \
        } \
    } while(0)

    FAILF_IF(argc != 3, "Expected exactly 2 arguments.\n");
    codeFileReader = NewBufferedFileReader(argv[1], OS_PAGE_SIZE);
    FAILF_IF(codeFileReader == NULL, "Failed to create reader for code file.\n");
    operationFile = fopen(argv[2], "w");
    FAILF_IF(operationFile == NULL, "Failed to open operation file with errcode: %d.\n", errno);
    FAILF_IF(!AssembleFile(codeFileReader, operationFile),  "Failed to assemble code.\n");

cleanup:
    if (operationFile != NULL) {
        if (fclose(operationFile) != 0) {
            ERROR("Failed to close operation file.");
        }
    }
    if (codeFileReader != NULL) {
        DeleteBufferedFileReader(codeFileReader);
    }
    return returnValue;
}

#undef FAILF_IF
