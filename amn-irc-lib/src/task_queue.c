#include "task_queue.h"

#include "queue.h"

TaskQueue* TaskQueue_New(size_t capacity, int32_t shutdownTimeout)
{
	return (TaskQueue*) Queue_New(capacity, shutdownTimeout, sizeof(Task*)); 
}

void TaskQueue_Delete(TaskQueue* self)
{
	Queue_Delete((Queue*) self);
}

bool TaskQueue_Push(TaskQueue* self, Task* task)
{
	return Queue_Push((Queue*) self, &task, sizeof(Task*));
}

Task* TaskQueue_Pop(TaskQueue* self)
{
	Task* task;

	if (!Queue_Pop((Queue*) self, &task, sizeof(Task*)))
	{
		return false;
	}

	return task;
}
