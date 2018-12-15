//
// Created by vladimirlesk on 08.12.18.
//

#ifndef IMACHINE_MACRO_H
#define IMACHINE_MACRO_H

#include <stdio.h>

#define ERROR(msg) \
  fputs(msg "\n", stderr)

#define ERRORF(...) fprintf(stderr, __VA_ARGS__)

#define RETURNF_IF(condition, returnValue, ...) \
    do { \
        if (condition) { \
            ERRORF(__VA_ARGS__); \
            return (returnValue); \
        } \
    } while(0)

#define OS_PAGE_SIZE (1u << 12u)

#endif //IMACHINE_MACRO_H
