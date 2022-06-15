#ifndef AMN_IRC_CMD_EXECUTOR_TASK_H
#define AMN_IRC_CMD_EXECUTOR_TASK_H

#include "log.h"
#include "task_queue.h"
#include "irc_cmd_queue.h"

/**
  * Task that executes the commands on the received IrcCmds.
  */
Task* IrcCmdExecutorTask_New(const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds);


#endif // AMN_IRC_CMD_EXECUTOR_TASK_H

