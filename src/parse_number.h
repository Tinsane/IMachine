//
// Created by vladimirlesk on 13.12.18.
//

#ifndef IMACHINE_PARSE_NUMBER_H
#define IMACHINE_PARSE_NUMBER_H

#include <stdint.h>

extern bool ParseUInt4(char const *string, uint16_t *result);

extern bool ParseUInt8(char const *string, uint16_t *result);

extern bool ParseUInt16(char const *string, uint16_t *result);

extern bool ParseUInt32(char const *string, uint32_t *result);

#endif //IMACHINE_PARSE_NUMBER_H
