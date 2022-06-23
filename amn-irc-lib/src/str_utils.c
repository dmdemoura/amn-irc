#include "str_utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char* StrUtils_Clone(const char* str)
{
	if (str == NULL)
	{
		return NULL;
	}

	size_t len = strlen(str) + 1;
	
	char* clone = malloc(sizeof(char) * len);
	if (clone == NULL)
	{
		return NULL;
	}

	memcpy(clone, str, sizeof(char) * len);

	return clone;
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
