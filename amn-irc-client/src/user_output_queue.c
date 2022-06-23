#include "user_output_queue.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

struct UserOutputQueue
{
	char** useroutputs;	
	size_t front; // The index of the next useroutput to be popped.
	size_t rear; // The index of the last pushed useroutputed.
	size_t capacity;

	pthread_mutex_t mutex;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
};

UserOutputQueue* UserOutputQueue_New(size_t capacity)
{
	UserOutputQueue* self = malloc(sizeof(UserOutputQueue));
	if (self == NULL)
		return NULL;

	self->front = SIZE_MAX; 
	self->rear = SIZE_MAX;
	self->capacity = capacity;

	self->useroutputs = malloc(sizeof(char*) * self->capacity);
	if (self->useroutputs == NULL)
		goto error_useroutputs;

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
	free(self->useroutputs);
error_useroutputs:
	free(self);
	return NULL;
}

void UserOutputQueue_Delete(UserOutputQueue* self)
{
	pthread_cond_destroy(&self->notFull);
	pthread_cond_destroy(&self->notEmpty);
	pthread_mutex_destroy(&self->mutex);

	free(self->useroutputs);
	free(self);
}

bool UserOutputQueue_IsEmpty(const UserOutputQueue* self)
{
	return self->front == SIZE_MAX;
}

bool UserOutputQueue_IsFull(const UserOutputQueue* self)
{
	return self->capacity == 0 || (self->rear + 1) % self->capacity == self->front;
}

bool UserOutputQueue_IsLastUserOutput(const UserOutputQueue* self)
{
	return self->front == self->rear;
}

bool UserOutputQueue_Push(UserOutputQueue* self, char* useroutput)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return false;
	}
	
	while(UserOutputQueue_IsFull(self))
	{
		if (pthread_cond_wait(&self->notFull, &self->mutex) != 0)
		{
			return false;
		}
	}

	if (UserOutputQueue_IsEmpty(self))
	{
		self->front = 0;
		self->rear = 0;
	}
	else
	{
		self->rear = (self->rear + 1) % self->capacity;
	}

	self->useroutputs[self->rear] = useroutput;

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

char* UserOutputQueue_Pop(UserOutputQueue* self)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return NULL;
	}

	while (UserOutputQueue_IsEmpty(self))
	{
		if (pthread_cond_wait(&self->notEmpty, &self->mutex) != 0 )
		{
			return NULL;
		}
	}

	char* useroutput = self->useroutputs[self->front];

	if (UserOutputQueue_IsLastUserOutput(self))
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
		return NULL;
	}
	if (pthread_mutex_unlock(&self->mutex) != 0)
	{
		return NULL;
	}

	return useroutput;
}

UserOutputQueue_TryPopResult UserOutputQueue_TryPop(UserOutputQueue* self, char** line)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return UserOutputQueue_TryPopResult_Error;
	}

	if (UserOutputQueue_IsEmpty(self))
	{
		return UserOutputQueue_TryPopResult_Empty;
	}

	char* useroutput = self->useroutputs[self->front];

	if (UserOutputQueue_IsLastUserOutput(self))
	{
		self->front = SIZE_MAX; 
		self->rear = SIZE_MAX;

		if (pthread_cond_signal(&self->notFull) != 0)
		{
			return UserOutputQueue_TryPopResult_Error;
		}
	}
	else
	{
		self->front = (self->front + 1) % self->capacity;
	}

	if (pthread_mutex_unlock(&self->mutex) != 0)
	{
		return UserOutputQueue_TryPopResult_Error;
	}

	*line = useroutput;

	return UserOutputQueue_TryPopResult_Ok;
}
