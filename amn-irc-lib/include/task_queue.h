#ifndef AMN_TASK_QUEUE_H
#define AMN_TASK_QUEUE_H

#include "task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct TaskQueue TaskQueue;

TaskQueue* TaskQueue_New(size_t capacity, int32_t shutdownTimeout);
void TaskQueue_Delete(TaskQueue* self);

bool TaskQueue_Push(TaskQueue* self, Task* task);
Task* TaskQueue_Pop(TaskQueue* self);


#endif // AMN_TASK_QUEUE_H
