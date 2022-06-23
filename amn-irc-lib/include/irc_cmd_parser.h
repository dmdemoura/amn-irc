#ifndef AMN_IRC_CMD_PARSER_H
#define AMN_IRC_CMD_PARSER_H

#include "log.h"
#include "irc_cmd.h"
#include "irc_msg_validator.h"

#include <stdbool.h>

typedef struct IrcCmdParser IrcCmdParser;

IrcCmdParser* IrcCmdParser_New(const Logger* logger, const IrcMsgValidator* validator);
void IrcCmdParser_Delete(IrcCmdParser* self);

IrcCmd* IrcCmdParser_Parse(IrcCmdParser* self, const IrcMsg* msg, int peerSocket);


#endif // AMN_IRC_CMD_PARSER_H
