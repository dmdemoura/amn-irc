#include "irc_cmd_queue.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

struct IrcCmdQueue
{
	IrcCmd** ircCmds;	
	size_t front; // The index of the next ircMsg to be popped.
	size_t rear; // The index of the last pushed ircMsged.
	size_t capacity;

	pthread_mutex_t mutex;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
};

IrcCmdQueue* IrcCmdQueue_New(size_t capacity)
{
	IrcCmdQueue* self = malloc(sizeof(IrcCmdQueue));
	if (self == NULL)
		return NULL;

	self->front = SIZE_MAX; 
	self->rear = SIZE_MAX;
	self->capacity = capacity;

	self->ircCmds = malloc(sizeof(IrcCmd*) * self->capacity);
	if (self->ircCmds == NULL)
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
	free(self->ircCmds);
error_ircMsgs:
	free(self);
	return NULL;
}

void IrcCmdQueue_Delete(IrcCmdQueue* self)
{
	pthread_cond_destroy(&self->notFull);
	pthread_cond_destroy(&self->notEmpty);
	pthread_mutex_destroy(&self->mutex);

	free(self->ircCmds);
	free(self);
}

bool IrcCmdQueue_IsEmpty(const IrcCmdQueue* self)
{
	return self->front == SIZE_MAX;
}

bool IrcCmdQueue_IsFull(const IrcCmdQueue* self)
{
	return self->capacity == 0 || (self->rear + 1) % self->capacity == self->front;
}

bool IrcCmdQueue_IsLastIrcCmd(const IrcCmdQueue* self)
{
	return self->front == self->rear;
}

bool IrcCmdQueue_Push(IrcCmdQueue* self, IrcCmd* ircCmd)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return false;
	}
	
	while(IrcCmdQueue_IsFull(self))
	{
		if (pthread_cond_wait(&self->notFull, &self->mutex) != 0)
		{
			return false;
		}
	}

	if (IrcCmdQueue_IsEmpty(self))
	{
		self->front = 0;
		self->rear = 0;
	}
	else
	{
		self->rear = (self->rear + 1) % self->capacity;
	}

	self->ircCmds[self->rear] = ircCmd;

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

IrcCmd* IrcCmdQueue_Pop(IrcCmdQueue* self)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return NULL;
	}

	while (IrcCmdQueue_IsEmpty(self))
	{
		if (pthread_cond_wait(&self->notEmpty, &self->mutex) != 0 )
		{
			return NULL;
		}
	}

	IrcCmd* ircCmd = self->ircCmds[self->front];

	if (IrcCmdQueue_IsLastIrcCmd(self))
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

	return ircCmd;
}
