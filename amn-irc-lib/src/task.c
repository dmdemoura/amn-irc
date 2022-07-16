#include "task.h"

#include <stdlib.h>

struct Task
{
	TaskStatus (*task)(void* context);
	void* context;	
	void (*deleteContext)(void* context);
};

Task* Task_Create(
	TaskStatus (*task)(void* context),
	void* context,	
	void (*deleteContext)(void* context))
{
	Task* self = malloc(sizeof(Task));

	if(self == NULL)
	{
		return NULL;
	}

	self->task = task;
	self->context = context;
	self->deleteContext = deleteContext;

	return self;
}

TaskStatus Task_Run(Task* self)
{
	return self->task(self->context);	
}

void Task_Delete(Task* self)
{
	if (self != NULL)
	{
		self->deleteContext(self->context);
		free(self);
	}
}
