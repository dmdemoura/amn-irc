#include "queue.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct Queue
{
	uint8_t* elements;	
	size_t front; // The index of the next ircMsg to be popped.
	size_t rear; // The index of the last pushed ircMsged.
	size_t capacity;
	int32_t shutdownTimeout;

	pthread_mutex_t mutex;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
};

Queue* Queue_New(size_t capacity, int32_t shutdownTimeout, size_t elementSize)
{
	Queue* self = malloc(sizeof(Queue));
	if (self == NULL)
		return NULL;

	self->front = SIZE_MAX; 
	self->rear = SIZE_MAX;
	self->capacity = capacity;
	self->shutdownTimeout = shutdownTimeout;

	self->elements = malloc(sizeof(elementSize) * self->capacity);
	if (self->elements == NULL)
		goto error_ircMsgs;

	if (pthread_mutex_init(&self->mutex, NULL) != 0)
		goto error_mutex;

	if (pthread_cond_init(&self->notEmpty, NULL) != 0)
		goto error_notEmpty;

	if (pthread_cond_init(&self->notFull, NULL) != 0)
		goto error_notFull;

	return self;

error_notFull:
	pthread_cond_destroy(&self->notEmpty);
error_notEmpty:
	pthread_mutex_destroy(&self->mutex);
error_mutex:
	free(self->elements);
error_ircMsgs:
	free(self);
	return NULL;
}

void Queue_Delete(Queue* self)
{
	pthread_cond_destroy(&self->notFull);
	pthread_cond_destroy(&self->notEmpty);
	pthread_mutex_destroy(&self->mutex);

	free(self->elements);
	free(self);
}

bool Queue_IsEmpty(const Queue* self)
{
	return self->front == SIZE_MAX;
}

bool Queue_IsFull(const Queue* self)
{
	return self->capacity == 0 || (self->rear + 1) % self->capacity == self->front;
}

bool Queue_IsLastElement(const Queue* self)
{
	return self->front == self->rear;
}

bool Queue_Push(Queue* self, void* element, size_t elementSize)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return false;
	}
	
	bool success = false;

	while(Queue_IsFull(self))
	{
		struct timespec abs_timeout;
		if (clock_gettime(CLOCK_REALTIME, &abs_timeout) == -1)
		{
			goto cleanup;
		}
		abs_timeout.tv_sec += self->shutdownTimeout;

		switch (pthread_cond_timedwait(&self->notFull, &self->mutex, &abs_timeout))
		{
			case 0: // Success
				break;
			case ETIMEDOUT:
				errno = EAGAIN;
			default:
				goto cleanup;
		}
	}

	if (Queue_IsEmpty(self))
	{
		self->front = 0;
		self->rear = 0;
	}
	else
	{
		self->rear = (self->rear + 1) % self->capacity;
	}

	memcpy(self->elements + self->rear * elementSize, element, elementSize);

	if (pthread_cond_signal(&self->notEmpty) != 0)
	{
		goto cleanup;
	}

	success = true;

cleanup:
	if (pthread_mutex_unlock(&self->mutex) != 0)
	{
		return false;
	}

	return success;
}

bool Queue_Pop(Queue* self, void* outElement, size_t elementSize)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return false;
	}

	bool success = false;

	while (Queue_IsEmpty(self))
	{
		struct timespec abs_timeout;
		if (clock_gettime(CLOCK_REALTIME, &abs_timeout) == -1)
		{
			goto cleanup;
		}
		abs_timeout.tv_sec += self->shutdownTimeout;

		switch (pthread_cond_timedwait(&self->notEmpty, &self->mutex, &abs_timeout))
		{
			case 0: // Success
				break;
			case ETIMEDOUT:
				errno = EAGAIN;
			default:
				goto cleanup;
		}
	}

	memcpy(outElement, self->elements + self->front * elementSize, elementSize);

	if (Queue_IsLastElement(self))
	{
		self->front = SIZE_MAX; 
		self->rear = SIZE_MAX;
	}
	else
	{
		self->front = (self->front + 1) % self->capacity;
	}

	if (pthread_cond_signal(&self->notFull) != 0)
	{
		goto cleanup;
	}

	success = true;

cleanup:
	if (pthread_mutex_unlock(&self->mutex) != 0)
	{
		return false;
	}

	return success;
}

Queue_TryPopResult Queue_TryPop(Queue* self, void* outElement, size_t elementSize)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return Queue_TryPopResult_Error;
	}

	Queue_TryPopResult result = Queue_TryPopResult_Error;

	if (Queue_IsEmpty(self))
	{
		result = Queue_TryPopResult_Empty;
		goto cleanup;
	}

	memcpy(&outElement, self->elements + self->front * elementSize, elementSize);

	if (Queue_IsLastElement(self))
	{
		self->front = SIZE_MAX; 
		self->rear = SIZE_MAX;
	}
	else
	{
		self->front = (self->front + 1) % self->capacity;
	}

	if (pthread_cond_signal(&self->notFull) != 0)
	{
		goto cleanup;
	}

	result = Queue_TryPopResult_Ok;

cleanup:
	if (pthread_mutex_unlock(&self->mutex) != 0)
	{
		return Queue_TryPopResult_Error;
	}

	return result;
}
