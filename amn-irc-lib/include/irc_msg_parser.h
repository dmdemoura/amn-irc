#ifndef AMN_IRC_MSG_PARSER_H
#define AMN_IRC_MSG_PARSER_H

#include "log.h"
#include "irc_msg.h"
#include "irc_msg_validator.h"

#include <stdbool.h>

/**
  * Note: The parser maintains internal state, and it's not thread-safe.
  *       Use one instance for each thread.
  */
typedef struct IrcMsgParser IrcMsgParser;

IrcMsgParser* IrcMsgParser_New(const Logger* logger, const IrcMsgValidator* validator);
void IrcMsgParser_Delete(IrcMsgParser* self);

IrcMsg* IrcMsgParser_Parse(IrcMsgParser* self, const char* rawMsg);


#endif // AMN_IRC_MSG_PARSER_H

