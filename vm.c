#include <stdio.h>
#include "src/executer.h"
#include "src/macro.h"
#include "src/parse_number.h"
#include "src/buffered_file_reader.h"

#define DEFAULT_TICKS_PER_ITER 10u

int main(int argc, char **argv) {
    int returnValue = 0;
    uint32_t iterationsPerProcess;
    struct Executer *executer = NULL;
    unsigned int subprocessCnt;
    unsigned int i;
    struct BufferedFileReader *subprocessFileReader;
    uint16_t subprocessCodeOffset;
    bool loadedSubprocess;

#define FAILF_IF(condition, ...) \
    do { \
        if (condition) { \
            ERRORF(__VA_ARGS__); \
            returnValue = 1; \
            goto cleanup; \
        } \
    } while(0)

    FAILF_IF((size_t) argc & 1u, "argc cannot be odd\n");
    subprocessCnt = (unsigned int) ((argc - 2) / 2);
    FAILF_IF(subprocessCnt == 0, "Should run at least one process\n");
    FAILF_IF(subprocessCnt > UINT16_MAX, "Cannot handle so many processes %u\n", subprocessCnt);
    FAILF_IF(!ParseUInt32(argv[1], &iterationsPerProcess),
             "Failed to parse number of iterations per process\n");
    executer = NewExecuter((uint16_t) subprocessCnt, DEFAULT_TICKS_PER_ITER);
    FAILF_IF(executer == NULL, "Failed to create an executer\n");

    for (i = 0; i < subprocessCnt; ++i) {
        FAILF_IF(!ParseUInt16(argv[3 + 2 * i], &subprocessCodeOffset),
                 "Failed to parse subprocess %u code offset\n", i);
        subprocessFileReader = NewBufferedFileReader(argv[2 + 2 * i], OS_PAGE_SIZE);
        FAILF_IF(subprocessFileReader == NULL, "Cannot open code for subprocess %u\n", i);
        loadedSubprocess = LoadSubprocess(executer, (uint16_t) i, subprocessCodeOffset,
                                          subprocessFileReader, iterationsPerProcess);
        DeleteBufferedFileReader(subprocessFileReader);
        FAILF_IF(!loadedSubprocess, "Failed to load subprocess %u\n", i);
    }
    FAILF_IF(!RunExecution(executer), "Failed to run execution\n");

    cleanup:
    if (executer != NULL) {
        DeleteExecuter(executer);
    }
    return returnValue;
}

#undef FAILF_IF
#undef DEFAULT_TICKS_PER_ITER
