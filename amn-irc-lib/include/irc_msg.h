#ifndef AMN_IRC_MSG_H
#define AMN_IRC_MSG_H

#include "irc_cmd_type.h"

#include <stddef.h>
#include <stdbool.h>

#define IRC_MSG_SIZE 512

// https://datatracker.ietf.org/doc/html/rfc1459#section-2.3
#define IRC_MSG_MAX_PARAMS 15

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

/**
 * Clones an IrcMsgPrefix.
 * On failure returns false, and the contents of clone are undefined. 
 */
bool IrcMsgPrefix_Clone(const IrcMsgPrefix* self, IrcMsgPrefix* clone);

// Parsed command-agnostic IRC message representation.
// https://datatracker.ietf.org/doc/html/rfc1459#section-2.3
typedef struct IrcMsg
{
	// Optional field, if missing prefix.origin will be null. 
	IrcMsgPrefix prefix;
	// Command type, either this or reply number must be present.
	IrcCmdType cmd;
	// Reply number, either this or command must be present.
	int replyNumber;
	// Command parameters, may be empty check paramCount.
	char* params[IRC_MSG_MAX_PARAMS];
	// Parameter count. If greater than zero, params cannot be null.
	size_t paramCount;
} IrcMsg;

IrcMsg* IrcMsg_Clone(const IrcMsg* self);
void IrcMsg_Delete(IrcMsg* self);

#endif // AMN_IRC_MSG_H

