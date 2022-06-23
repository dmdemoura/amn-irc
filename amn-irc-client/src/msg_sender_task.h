#ifndef AMN_MSG_SENDER_TASK_H
#define AMN_MSG_SENDER_TASK_H

#include "log.h"
#include "task.h"
#include "user_input_queue.h"

Task* MsgSenderTask_New(const Logger* log, UserInputQueue* userInput, int socket); 

#endif /// AMN_MSG_SENDER_TASK_H
