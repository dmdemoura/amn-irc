#ifndef AMN_IRC_MSG_WRITER_H
#define AMN_IRC_MSG_WRITER_H

#include "log.h"

#include <stdbool.h>

typedef struct IrcMsgWriter IrcMsgWriter;

IrcMsgWriter* IrcMsgWriter_New(const Logger* log, int socket);
void IrcMsgWriter_Delete(IrcMsgWriter* self);

bool IrcMsgWriter_Write(IrcMsgWriter* self, const char* msg);

#endif // AMN_IRC_MSG_WRITER_H
