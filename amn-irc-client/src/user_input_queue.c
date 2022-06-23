#include "user_input_queue.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

struct UserInputQueue
{
	char** userinputs;	
	size_t front; // The index of the next userinput to be popped.
	size_t rear; // The index of the last pushed userinputed.
	size_t capacity;

	pthread_mutex_t mutex;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
};

UserInputQueue* UserInputQueue_New(size_t capacity)
{
	UserInputQueue* self = malloc(sizeof(UserInputQueue));
	if (self == NULL)
		return NULL;

	self->front = SIZE_MAX; 
	self->rear = SIZE_MAX;
	self->capacity = capacity;

	self->userinputs = malloc(sizeof(char*) * self->capacity);
	if (self->userinputs == NULL)
		goto error_userinputs;

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
	free(self->userinputs);
error_userinputs:
	free(self);
	return NULL;
}

void UserInputQueue_Delete(UserInputQueue* self)
{
	pthread_cond_destroy(&self->notFull);
	pthread_cond_destroy(&self->notEmpty);
	pthread_mutex_destroy(&self->mutex);

	free(self->userinputs);
	free(self);
}

bool UserInputQueue_IsEmpty(const UserInputQueue* self)
{
	return self->front == SIZE_MAX;
}

bool UserInputQueue_IsFull(const UserInputQueue* self)
{
	return self->capacity == 0 || (self->rear + 1) % self->capacity == self->front;
}

bool UserInputQueue_IsLastUserInput(const UserInputQueue* self)
{
	return self->front == self->rear;
}

bool UserInputQueue_Push(UserInputQueue* self, char* userinput)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return false;
	}
	
	while(UserInputQueue_IsFull(self))
	{
		if (pthread_cond_wait(&self->notFull, &self->mutex) != 0)
		{
			return false;
		}
	}

	if (UserInputQueue_IsEmpty(self))
	{
		self->front = 0;
		self->rear = 0;
	}
	else
	{
		self->rear = (self->rear + 1) % self->capacity;
	}

	self->userinputs[self->rear] = userinput;

	if (pthread_cond_signal(&self->notEmpty) != 0)
	{
		return false;
	}
	if (pthread_mutex_unlock(&self->mutex) != 0)
	{
		return false;
	}

	return true;
}

char* UserInputQueue_Pop(UserInputQueue* self)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return NULL;
	}

	while (UserInputQueue_IsEmpty(self))
	{
		if (pthread_cond_wait(&self->notEmpty, &self->mutex) != 0 )
		{
			return NULL;
		}
	}

	char* userinput = self->userinputs[self->front];

	if (UserInputQueue_IsLastUserInput(self))
	{
		self->front = SIZE_MAX; 
		self->rear = SIZE_MAX;

		if (pthread_cond_signal(&self->notFull) != 0)
		{
			return NULL;
		}
	}
	else
	{
		self->front = (self->front + 1) % self->capacity;
	}

	if (pthread_mutex_unlock(&self->mutex) != 0)
	{
		return NULL;
	}

	return userinput;
}
