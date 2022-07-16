#ifndef AMN_IRC_CMD_QUEUE_H
#define AMN_IRC_CMD_QUEUE_H

#include "irc_cmd.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct IrcCmdQueue IrcCmdQueue;

IrcCmdQueue* IrcCmdQueue_New(size_t capacity, int32_t shutdownTimeout);
void IrcCmdQueue_Delete(IrcCmdQueue* self);

bool IrcCmdQueue_Push(IrcCmdQueue* self, IrcCmd* ircCmd);
IrcCmd* IrcCmdQueue_Pop(IrcCmdQueue* self);


#endif // AMN_IRC_CMD_QUEUE_H
