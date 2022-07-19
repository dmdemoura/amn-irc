#ifndef AMN_ARRAY_LIST_H
#define AMN_ARRAY_LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct ArrayList ArrayList;

ArrayList* ArrayList_New(size_t initialSize, size_t expansionSize, size_t elementSize,
		void deleteElement(void* element));

bool ArrayList_Append(ArrayList* self, const void* element);

void ArrayList_Insert(ArrayList* self, const void* element, size_t index);

void ArrayList_Set(ArrayList* self, const void* element, size_t index);

void* ArrayList_Get(const ArrayList* self, size_t index);

void* ArrayList_Find(
		ArrayList* self, bool compare(const void*, const void*), const void* compareData);

size_t ArrayList_FindIndex(
		ArrayList* self, bool compare(const void*, const void*), const void* compareData);

size_t ArrayList_Size(ArrayList* self);

bool ArrayList_Remove(
		ArrayList* self, bool compare(const void*, const void*), const void* compareData,
		bool delete);
bool ArrayList_RemoveIndex(ArrayList* self, size_t index, bool delete);

void ArrayList_Print(ArrayList* self, void printData(void*));
void ArrayList_Clear(ArrayList* self);
void ArrayList_Delete(ArrayList* self);


#endif // AMN_ARRAY_LIST_H
