#ifndef AMN_IRC_MSG_UNPARSER_H
#define AMN_IRC_MSG_UNPARSER_H

#include "log.h"
#include "irc_msg.h"

/**
  * Note: The unparser maintains internal state, and it's not thread-safe.
  *       Use one instance for each thread.
  */
typedef struct IrcMsgUnparser IrcMsgUnparser;

IrcMsgUnparser* IrcMsgUnparser_New(const Logger* log);
void IrcMsgUnparser_Delete(IrcMsgUnparser* self);

/**
 * Unparses a message into a raw string.
 * Note: the returned pointer is owned by the Unparser instance.
 */
const char* IrcMsgUnparser_Unparse(IrcMsgUnparser* self, const IrcMsg* msg);

#endif // AMN_IRC_MSG_UNPARSER_H

