#include "task_queue.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

struct TaskQueue
{
	Task** tasks;	
	size_t front; // The index of the next task to be popped.
	size_t rear; // The index of the last pushed tasked.
	size_t capacity;

	pthread_mutex_t mutex;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
};

TaskQueue* TaskQueue_New(size_t capacity)
{
	TaskQueue* self = malloc(sizeof(TaskQueue));
	if (self == NULL)
		return NULL;

	self->front = SIZE_MAX; 
	self->rear = SIZE_MAX;
	self->capacity = capacity;

	self->tasks = malloc(sizeof(Task*) * self->capacity);
	if (self->tasks == NULL)
		goto error_tasks;

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
	free(self->tasks);
error_tasks:
	free(self);
	return NULL;
}

void TaskQueue_Delete(TaskQueue* self)
{
	pthread_cond_destroy(&self->notFull);
	pthread_cond_destroy(&self->notEmpty);
	pthread_mutex_destroy(&self->mutex);

	free(self->tasks);
	free(self);
}

bool TaskQueue_IsEmpty(const TaskQueue* self)
{
	return self->front == SIZE_MAX;
}

bool TaskQueue_IsFull(const TaskQueue* self)
{
	return self->capacity == 0 || (self->rear + 1) % self->capacity == self->front;
}

bool TaskQueue_IsLastTask(const TaskQueue* self)
{
	return self->front == self->rear;
}

bool TaskQueue_Push(TaskQueue* self, Task* task)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return false;
	}
	
	while(TaskQueue_IsFull(self))
	{
		if (pthread_cond_wait(&self->notFull, &self->mutex) != 0)
		{
			return false;
		}
	}

	if (TaskQueue_IsEmpty(self))
	{
		self->front = 0;
		self->rear = 0;
	}
	else
	{
		self->rear = (self->rear + 1) % self->capacity;
	}

	self->tasks[self->rear] = task;

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

Task* TaskQueue_Pop(TaskQueue* self)
{
	if (pthread_mutex_lock(&self->mutex) != 0)
	{
		return NULL;
	}

	while (TaskQueue_IsEmpty(self))
	{
		if (pthread_cond_wait(&self->notEmpty, &self->mutex) != 0 )
		{
			return NULL;
		}
	}

	Task* task = self->tasks[self->front];

	if (TaskQueue_IsLastTask(self))
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

	return task;
}


