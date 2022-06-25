#ifndef AMN_IRC_CMD_UNPARSER_H
#define AMN_IRC_CMD_UNPARSER_H

#include "log.h"
#include "irc_cmd.h"
#include "irc_msg_validator.h"

#include <stdbool.h>

typedef struct IrcCmdUnparser IrcCmdUnparser;

IrcCmdUnparser* IrcCmdUnparser_New(const Logger* logger, const IrcMsgValidator* validator);
void IrcCmdUnparser_Delete(IrcCmdUnparser* self);

IrcMsg* IrcCmdUnparser_Unparse(IrcCmdUnparser* self, const IrcCmd* cmd);

#endif // AMN_IRC_CMD_UNPARSER_H
