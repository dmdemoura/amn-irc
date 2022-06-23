#ifndef AMN_ACCEPT_CONN_TASK_H
#define AMN_ACCEPT_CONN_TASK_H

#include "log.h"
#include "task.h"
#include "task_queue.h"
#include "irc_cmd_queue.h"

/**
  * Task to accept incoming connection from a socket.
  * Accepted connections are added to the TaskQueue.
  */
Task* AcceptConnTask_New(const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, int socket);


#endif // AMN_ACCEPT_CONN_TASK_H

