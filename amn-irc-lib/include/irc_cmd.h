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

typedef enum IrcChannelType
{
	IrcChannelType_Local,
	IrcChannelType_Distributed
}
IrcChannelType;

typedef struct IrcChannelAndKey
{
	char* name;
	char* key;
	IrcChannelType type;
}
IrcChannelAndKey;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.2.1
typedef struct IrcCmdJoin
{
	IrcChannelAndKey* channels;	
	size_t channelCount;
}
IrcCmdJoin;

typedef struct IrcChannel
{
	char* name;
	IrcChannelType type;
}
IrcChannel;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.2.2
typedef struct IrcCmdPart
{
	IrcChannel* channels;	
}
IrcCmdPart;


typedef enum IrcCmdModeTarget
{
	IrcCmdModeTarget_Channel,
	IrcCmdModeTarget_User
}
IrcCmdModeTarget;

// IrcMode bit flags
typedef enum IrcMode
{
	IrcMode_None						= 0,
	IrcMode_Channel_Private				= 1,	// p
	IrcMode_Channel_Secret				= 2,	// s
	IrcMode_Channel_InviteOnly			= 4,	// i
	IrcMode_Channel_TopicLocked			= 8,	// t
	IrcMode_Channel_NoExternalMessages	= 16,	// n
	IrcMode_Channel_Moderated			= 32,	// m
	IrcMode_Channel_LimitedUsers		= 64,	// l
	IrcMode_Channel_BanMasked			= 128,	// b
	IrcMode_Channel_RequiresKey			= 256,	// k
	IrcMode_ChannelUser_Operator		= 512,	// o
	IrcMode_ChannelUser_Voiced			= 1024,	// v
	IrcMode_User_Invisible				= 2048,	// i
	IrcMode_User_ReceiveServerNotices	= 4096,	// s
	IrcMode_User_ReceiveWallops			= 8192,	// w,
	IrcMode_User_Operator				= 16384	// o
}
IrcMode;

// Type that can store a mask of IrcModes
typedef int32_t IrcModes;

// https://datatracker.ietf.org/doc/html/rfc1459#section-4.2.3
typedef struct IrcCmdMode
{
	// User or Channel
	IrcCmdModeTarget targetType;
	// User nickname or Channel name.
	char* targetName;
	IrcModes modes;
	size_t limit;
	char* nickname;
	char* banmask;
}
IrcCmdMode;


// https://datatracker.ietf.org/doc/html/rfc1459#section-4.2.8
typedef struct IrcCmdKick
{
	IrcChannel channel;
	char* nickname;
}
IrcCmdKick;

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
		IrcCmdJoin join;
		IrcCmdPart part;
		IrcCmdMode mode;
		IrcCmdKick kick;
		IrcCmdPrivMsg privMsg;
		// TODO: Add missing commands
	};
} IrcCmd;

IrcCmd* IrcCmd_Clone(const IrcCmd* self);

void IrcCmd_Delete(IrcCmd* self);

#endif // AMN_IRC_CMD_H
