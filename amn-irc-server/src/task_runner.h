#ifndef AMN_TASK_RUNNER_H
#define AMN_TASK_RUNNER_H

#include "log.h"
#include "task_queue.h"

typedef struct TaskRunner TaskRunner;

TaskRunner* TaskRunner_New(Logger* log, TaskQueue* tasks);
void TaskRunner_Delete(TaskRunner* self);

#endif // AMN_TASK_RUNNER_H

