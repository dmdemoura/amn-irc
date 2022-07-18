#ifndef AMN_QUEUE_H
#define AMN_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Queue Queue;

typedef enum Queue_TryPopResult 
{
	Queue_TryPopResult_Ok,
	Queue_TryPopResult_Empty,
	Queue_TryPopResult_Error,
}
Queue_TryPopResult;

Queue* Queue_New(size_t capacity, int32_t shutdownTimeout, size_t elementSize);
void Queue_Delete(Queue* self, void (*elementDeleter)(void*), size_t elementSize);

bool Queue_Push(Queue* self, void* element, size_t elementSize);
bool Queue_Pop(Queue* self, void* outElement, size_t elementSize);
Queue_TryPopResult Queue_TryPop(Queue* self, void* outElement, size_t elementSize);

#endif // AMN_QUEUE_H
