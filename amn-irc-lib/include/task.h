#ifndef AMN_TASK_H
#define AMN_TASK_H

typedef struct Task Task;

typedef enum TaskStatus
{
	/**
	 * The task is not done and should continue to be executed.
	 */
	TaskStatus_Yield,
	/**
	 * The task is done and should not continue to be executed.
	 */
	TaskStatus_Done,
	/**
	 * The task has failed and should not continue to be executed.
	 */
	TaskStatus_Failed,
}
TaskStatus;

Task* Task_Create(
	TaskStatus (*task)(void* context),
	void* context,	
	void (*deleteContext)(void* context));

TaskStatus Task_Run(Task* self);

void Task_Delete(Task* self);

#endif

