#include "array_list.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ArrayList
{
	void* elements;
	size_t currentSize;
	size_t allocatedSize;
	size_t expandSize;
	size_t elementSize;
	void (*deleteElement)(void* element);
};

static bool ArrayList_Expand(ArrayList* self)
{
	size_t newSize = self->allocatedSize + self->expandSize;
	void** elements = realloc(self->elements, newSize * sizeof(void*));
	if (!elements)
	{
		return false;
	}

	self->elements = elements;
	self->allocatedSize = newSize;

	return true;
}

ArrayList* ArrayList_New(size_t initialSize, size_t expansionSize, size_t elementSize,
		void deleteElement(void* element))
{
	ArrayList* self = malloc(sizeof(ArrayList));
	if(!self)
	{
		return NULL;
	}


	self->elements = malloc(initialSize * elementSize);
	if (!self->elements)
	{
		free(self);
		return NULL;
	}

	self->currentSize = 0;
	self->allocatedSize = initialSize;
	self->expandSize = expansionSize;
	self->elementSize = elementSize;
	self->deleteElement = deleteElement;

	return self;
}

bool ArrayList_Append(ArrayList* self, const void* element)
{
	if (self->currentSize == self->allocatedSize)
	{
		if (!ArrayList_Expand(self))
		{
			return false;
		}
	}


	ArrayList_Set(self, element, self->currentSize);
	self->currentSize++;

	return true;
}

void ArrayList_Insert(ArrayList* self, const void* element, size_t index);

void ArrayList_Set(ArrayList* self, const void* element, size_t index)
{
	memcpy((char*) self->elements + index * self->elementSize, element, self->elementSize); 
}

void* ArrayList_Get(const ArrayList* self, size_t index)
{
   return (char*) self->elements + index * self->elementSize;
}

void* ArrayList_Find(
		ArrayList* self, bool compare(const void*, const void*), const void* compareData)
{
	for (size_t i = 0; i < self->currentSize; i++)
	{
		void* element = ArrayList_Get(self, i);
		if (compare(element, compareData))
		{
			return element;
		}
	}

	return NULL;
}

size_t ArrayList_FindIndex(
		ArrayList* self, bool compare(const void*, const void*), const void* compareData)
{
	for (size_t i = 0; i < self->currentSize; i++)
	{
		if (compare(ArrayList_Get(self, i), compareData))
		{
			return i;
		}
	}

	return SIZE_MAX;
}

size_t ArrayList_Size(ArrayList* self)
{
	return self->currentSize;
}

bool ArrayList_Remove(
		ArrayList* self, bool compare(const void*, const void*), const void* compareData);
// {
// 	size_t i = ArrayList_FindIndex(self, compare, compareData);
// 	if (i == SIZE_MAX)
// 	{
// 		return false;
// 	}

// 	return ArrayList_RemoveIndex(self, i);
// }

bool ArrayList_RemoveIndex(ArrayList* self, size_t index);

void ArrayList_Print(ArrayList* self, void printData(void*));
void ArrayList_Delete(ArrayList* self)
{
	if (self == NULL)
	{
		return;
	}

	for (size_t i = 0; i < self->currentSize; i++)
	{
		self->deleteElement(ArrayList_Get(self, i));
	}

	free(self);
}
