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

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.6
typedef struct IrcCmdQuit
{
	char* quitMessage;
} IrcCmdQuit;

typedef enum IrcReceiverType
{
	IrcReceiverType_Nickname,
	IrcReceiverType_LocalChannel,
	// Yay ambiguity!
	IrcReceiverType_DistChannelOrHostMask,
	IrcReceiverType_ServerMask,
}
IrcReceiverType;

typedef struct IrcReceiver
{
	IrcReceiverType type;
	char* value;
}
IrcReceiver;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.4.1
typedef struct IrcCmdPrivMsg
{
	IrcReceiver* receiver;
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
		IrcCmdQuit quit;
		IrcCmdPrivMsg privMsg;
		// TODO: Add missing commands
	};
} IrcCmd;

IrcCmd* IrcCmd_Clone(const IrcCmd* self);

void IrcCmd_Delete(IrcCmd* self);

#endif // AMN_IRC_CMD_H
