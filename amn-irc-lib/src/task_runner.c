#include "task_runner.h"

#include "application.h"
#include "task.h"
#include "task_queue.h"

#include <errno.h>
#include <stdlib.h>

#include <pthread.h>

struct TaskRunner
{
	const Logger* log;
	TaskQueue* tasks;
	pthread_t thread;
};

static void* TaskRunner_Run(void* self);

TaskRunner* TaskRunner_New(Logger* log, TaskQueue* taskQueue)
{
	TaskRunner* self = malloc(sizeof(TaskRunner));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocate TaskRunner");
		return NULL;
	}

	self->log = log;
	self->tasks = taskQueue;

	if(pthread_create(&self->thread, NULL, TaskRunner_Run, self) != 0)
	{
		LOG_ERROR(self->log, "Failed to create TaskRunner: thread creation failed.");
		return NULL;
	}

	return self;
}

void TaskRunner_Delete(TaskRunner* self)
{
	if (self == NULL)
	{
		return;
	}

	if(pthread_join(self->thread, NULL) != 0)
	{
		LOG_ERROR(self->log, "Failed to stop TaskRunner: thread join failed.");
	}

	LOG_DEBUG(self->log, "Task runner shut down.");

	free(self);
}

static void* TaskRunner_Run(void* arg)
{
	TaskRunner* self = (TaskRunner*) arg;

	while (!Application_ShouldShutdown())
	{
		Task* task = TaskQueue_Pop(self->tasks);
		if (task == NULL && errno == EAGAIN)
		{
			continue;
		}
		else if (task == NULL)
		{
			LOG_ERROR(self->log, "Failed to get task from queue!");
			return NULL;
		}

		TaskStatus status;
		do
		{
			status = Task_Run(task);
		}
		while (status == TaskStatus_Yield && !Application_ShouldShutdown());

		Task_Delete(task);
	}

	errno = 0;

	LOG_INFO(self->log, "Task runner shutting down.");

	return NULL;
}
