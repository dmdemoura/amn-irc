#ifndef AMN_IRC_CMD_H
#define AMN_IRC_CMD_H

#include "irc_msg.h"

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.2
typedef struct IrcCmdNick
{
	IrcMsgPrefix prefix;
	char* nickname;
	size_t hopCount;
} IrcCmdNick;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.3
typedef struct IrcCmdUser
{
	IrcMsgPrefix prefix;
	char* username;
	char* hostname;
	char* servername;
	char* realname;
} IrcCmdUser;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.4.1
typedef struct IrcCmdPrivMsg
{
	IrcMsgPrefix prefix;
	char** receiver;
	char* text;
} IrcCmdPrivMsg; 

// Generic command structure
typedef struct IrcCmd
{
	IrcCmdType type;
	int peerSocket;
	union {
		IrcCmdNick nick;
		IrcCmdUser user;
		IrcCmdPrivMsg privMsg;
		// TODO: Add missing commands
	};
} IrcCmd;

void IrcCmd_Delete(IrcCmd* self);

#endif // AMN_IRC_CMD_H
