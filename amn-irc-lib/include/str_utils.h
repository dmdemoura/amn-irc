#ifndef AMN_STR_UTILS_H
#define AMN_STR_UTILS_H

#include <stddef.h>
#include <stdbool.h>

const char* StrUtils_SkipCharacter(const char* str, char charToSkip);

char* StrUtils_Clone(const char* str);

char* StrUtils_CloneRange(const char* start, const char* end);

bool StrUtils_ReadSizeT(const char* str, size_t* value);

const char* StrUtils_FindFirst(const char* string, const char* charsToFind);

#endif // AMN_STR_UTILS_H

