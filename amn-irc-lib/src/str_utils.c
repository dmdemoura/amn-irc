#include "str_utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char* StrUtils_Clone(const char* str)
{
	return StrUtils_CloneRange(str, NULL);
}

char* StrUtils_CloneRange(const char* start, const char* end)
{
	if (start == NULL)
	{
		return NULL;
	}

	size_t len;
	if (end != NULL)
	{
		ptrdiff_t diff = end - start;

		if (diff < 0)
		{
			return false;
		}

		len = (size_t) diff; 
	}
	else
	{
		len = strlen(start);
	}

	char* clone = malloc(sizeof(char) * (len + 1));
	if (clone == NULL)
	{
		return NULL;
	}

	memcpy(clone, start, len);
	clone[len] = '\0';

	return clone;
}

bool StrUtils_Equals(const char* str, const char* other)
{
	if (str == other)
	{
		return true;
	}

	if (str == NULL || other == NULL)
	{
		return false;
	}

	return strcmp(str, other) == 0;
}

bool StrUtils_ReadSizeT(const char* str, size_t* value)
{
	if (*str == '\0')
	{
		return false;
	}

	*value = 0;

	for (; *str != '\0'; str++)
	{
		if (*str < '0' || *str > '9')
		{
			return false;
		}

		size_t digit = (size_t) *str - (size_t) '0';

		if (*value > SIZE_MAX / 10)
		{
			// Multiplication would overflow
			return false;
		}

		*value *= 10;

		if (*value > SIZE_MAX - digit)
		{
			// Addition would overflow
			return false;
		}

		*value += digit;
	}

	return true;
}

const char* StrUtils_FindFirst(const char* string, const char* charsToFind)
{
	for (; *string != '\0'; string += 1)
	{
		for(const char* c = charsToFind; *c != '\0'; c += 1)
		{
			if (*string == *c)
			{
				return string;
			}
		}
	}

	return NULL;
}
