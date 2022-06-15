#ifndef AMN_RECEIVE_MSG_TASK_H
#define AMN_RECEIVE_MSG_TASK_H

#include "log.h"
#include "task.h"
#include "task_queue.h"

/**
  * Task to reading incoming messages from one client. 
  */
Task* ReceiveMsgTask_New(const Logger* log, TaskQueue* tasks, int socket);

#endif // AMN_RECEIVE_MSG_TASK_H

