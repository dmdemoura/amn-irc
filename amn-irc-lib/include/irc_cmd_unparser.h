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

IrcMsg* IrcCmdUnparser_UnparseNick(IrcCmdUnparser* self,
		int peerSocket,
		const char* nickname,
		size_t hopCount);

IrcMsg* IrcCmdUnparser_UnparseUser(IrcCmdUnparser* self,
		int peerSocket,
		const char* username,
		const char* hostname,
		const char* servername,
		const char* realname);

IrcMsg* IrcCmdUnparser_UnparsePrivMsg(IrcCmdUnparser* self,
		int peerSocket,
		const char* const* receiver,
		size_t receiverCount,
		const char* text);

#endif // AMN_IRC_CMD_UNPARSER_H
