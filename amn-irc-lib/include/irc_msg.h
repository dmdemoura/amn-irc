#ifndef AMN_IRC_MSG_H
#define AMN_IRC_MSG_H

#include "irc_cmd_type.h"

#include <stddef.h>


// https://datatracker.ietf.org/doc/html/rfc1459#section-2.3
typedef struct IrcMsgPrefix
{
	// The origin of the message, either a servername or nickname depending on the command.
	// It it always present if a prefix exists.
	char* origin;
	// If origin is an nickname, the client's username may optionally be present.
	char* username;
	// If origin is an nickname, the client's hostname may optionally be present.
	char* hostname;
} IrcMsgPrefix;

// Parsed command-agnostic IRC message representation.
// https://datatracker.ietf.org/doc/html/rfc1459#section-2.3
typedef struct IrcMsg
{
	// Optional field, if missing prefix.origin will be null. 
	IrcMsgPrefix prefix;
	// Command type, always present.
	IrcCmdType cmd;
	// Command parameters, may be empty check paramCount.
	char** params;
	// Parameter count. If greater than zero, params cannot be null.
	size_t paramCount;
} IrcMsg;

#endif // AMN_IRC_MSG_H

