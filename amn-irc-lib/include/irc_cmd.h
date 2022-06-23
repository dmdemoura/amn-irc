#ifndef AMN_IRC_CMD_H
#define AMN_IRC_CMD_H

#include "irc_msg.h"

#define IRC_CMD_PRIVMSG_MAX_RECEIVERS 14

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.2
typedef struct IrcCmdNick
{
	char* nickname;
	size_t hopCount;
} IrcCmdNick;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.3
typedef struct IrcCmdUser
{
	char* username;
	char* hostname;
	char* servername;
	char* realname;
} IrcCmdUser;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.4.1
typedef struct IrcCmdPrivMsg
{
	char* receiver[IRC_CMD_PRIVMSG_MAX_RECEIVERS];
	size_t receiverCount;
	char* text;
} IrcCmdPrivMsg; 

// Generic command structure
typedef struct IrcCmd
{
	IrcCmdType type;
	IrcMsgPrefix prefix;
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
