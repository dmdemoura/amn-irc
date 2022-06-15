#ifndef AMN_TASK_H
#define AMN_TASK_H

typedef struct Task Task;

Task* Task_Create(
	void (*task)(void* context),
	void* context,	
	void (*deleteContext)(void* context));

void Task_Run(Task* self);

void Task_Delete(Task* self);

#endif

