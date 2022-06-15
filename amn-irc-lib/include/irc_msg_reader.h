#ifndef AMN_IRC_MSG_READER_H
#define AMN_IRC_MSG_READER_H

#include "log.h"

typedef struct IrcMsgReader IrcMsgReader;

IrcMsgReader* IrcMsgReader_New(const Logger* log, int socket);
void IrcMsgReader_Delete(IrcMsgReader* self);

const char* IrcMsgReader_Read(IrcMsgReader* self);

#endif // AMN_IRC_MSG_READER_H
