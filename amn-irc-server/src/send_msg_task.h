#ifndef AMN_SEND_MSG_TASK_H
#define AMN_SEND_MSG_TASK_H

#include "log.h"
#include "task.h"

/**
  * Task to send an message. 
  */
Task* SendMsgTask_New(const Logger* log, int socket, const char* msg);


#endif // AMN_SEND_MSG_TASK_H
