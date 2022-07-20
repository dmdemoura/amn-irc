#include "irc_msg_parser.h"

#include "irc_cmd_map.h"
#include "str_utils.h"

#include <stdlib.h>
#include <string.h>


struct IrcMsgParser
{
	const Logger* log;
	const IrcMsgValidator* validator;
	IrcMsg* msg;
	const char* rawMsg;
};


IrcMsgParser* IrcMsgParser_New(const Logger* logger, const IrcMsgValidator* validator)
{
	IrcMsgParser* self = malloc(sizeof(IrcMsgParser));
	if (self == NULL)
	{
		return NULL;
	}

	self->log = logger;
	self->validator = validator;
	self->msg = NULL;
	self->rawMsg = NULL;

	return self;
}

void IrcMsgParser_Delete(IrcMsgParser* self)
{
	free(self);
}

static bool IrcMsgParser_ParsePrefixOrigin(IrcMsgParser* self)
{
	const char* originStart = self->rawMsg;
	const char* originEnd = StrUtils_FindFirst(originStart, "!@ ");

	// Advance buffer
	self->rawMsg = originEnd;

	if (originEnd == NULL)
	{
		LOG_WARN(self->log,
				"Invalid message: expected '!', '@' or <SPACE> after <prefix>'s origin.");

		return false;
	}

	if (!IrcMsgValidator_ValidateOrigin(self->validator, originStart, originEnd))
	{
		return false;
	}

	self->msg->prefix.origin = StrUtils_CloneRange(originStart, originEnd);
	if (self->msg->prefix.origin == NULL)
	{
		LOG_ERROR(self->log, "Failed to copy IrcMsgPrefix.origin");
		return false;
	}

	return true;
}

static bool IrcMsgParser_ParsePrefixUsername(IrcMsgParser* self)
{
	if (*self->rawMsg != '!')
	{
		// There's no prefix username.
		return true;
	}

	const char* usernameStart = self->rawMsg + 1;
	const char* usernameEnd = StrUtils_FindFirst(usernameStart, "@ ");

	// Advance buffer
	self->rawMsg = usernameEnd;

	if (usernameEnd == NULL)
	{
		LOG_WARN(self->log,
				"Invalid message: expected '@' or <SPACE> after <prefix>'s <user>.");

		return false;
	}

	if (!IrcMsgValidator_ValidateUser(self->validator, usernameStart, usernameEnd))
	{
		return false;
	}

	self->msg->prefix.username = StrUtils_CloneRange(usernameStart, usernameEnd);
	if (self->msg->prefix.username == NULL)
	{
		LOG_ERROR(self->log, "Failed to copy IrcMsgPrefix.username");
		return false;
	}

	return true;
}

static bool IrcMsgParser_ParsePrefixHostname(IrcMsgParser* self)
{
	if (*self->rawMsg != '@')
	{
		// There's no prefix hostname.
		return true;
	}

	const char* hostnameStart = self->rawMsg + 1;
	const char* hostnameEnd = strchr(hostnameStart, ' ');

	// Advance buffer
	self->rawMsg = hostnameEnd;

	if (hostnameEnd == NULL)
	{
		LOG_WARN(self->log,
				"Invalid message: expected <SPACE> after <prefix>'s <host>.");

		return false;
	}

	if (!IrcMsgValidator_ValidateHost(self->validator, hostnameStart, hostnameEnd))
	{
		return false;
	}

	self->msg->prefix.hostname = StrUtils_CloneRange(hostnameStart, hostnameEnd);
	if (self->msg->prefix.hostname == NULL)
	{
		LOG_ERROR(self->log, "Failed to copy IrcMsgPrefix.hostname");
		return false;
	}

	return true;
}

static bool IrcMsgParser_ParsePrefix(IrcMsgParser* self)
{
	if (*self->rawMsg != ':')
	{
		// Msg has no prefix, continue.
		return true;
	}
	self->rawMsg += 1;


	if (!IrcMsgParser_ParsePrefixOrigin(self))
	{
		LOG_WARN(self->log, "Failed to parse <prefix>'s origin");
		return false;
	}

	if (!IrcMsgParser_ParsePrefixUsername(self))
	{
		LOG_WARN(self->log, "Failed to parse <prefix>'s <user>");
		return false;
	}

	if (!IrcMsgParser_ParsePrefixHostname(self))
	{
		LOG_WARN(self->log, "Failed to parse <prefix>'s <host>");
		return false;
	}

	return true;
}

static void IrcMsgParser_ParseSpace(IrcMsgParser* self)
{
	while(*self->rawMsg == ' ')
	{
		self->rawMsg += 1;
	}
}

static bool IrcMsgParser_ParseCommand(IrcMsgParser* self)
{
	const char* cmdStart = self->rawMsg;
	const char* cmdEnd = strchr(cmdStart, ' ');

	self->rawMsg = cmdEnd;

	if (cmdEnd == NULL)
	{
		LOG_WARN(self->log,
				"Invalid message: expected <SPACE> after <command>.");

		return false;
	}

	if (!IrcMsgValidator_ValidateCommand(self->validator, cmdStart, cmdEnd))
	{
		return false;
	}

	IrcCmdType cmd = IrcCmdType_FromStr(cmdStart, (size_t) (cmdEnd - cmdStart));
	if (cmd == IrcCmdType_Null)
		return false;

	self->msg->cmd = cmd;
	return true;
}

static bool IrcMsgParser_ParseMiddleParam(IrcMsgParser* self)
{
	const char* paramStart = self->rawMsg;
	const char* paramEnd = StrUtils_FindFirst(paramStart, " \r");

	self->rawMsg = paramEnd;

	if (paramEnd == NULL)
	{
		LOG_WARN(self->log,
				"Invalid message: expected <SPACE> or <CR> after <middle>.");

		return false;
	}

	if (!IrcMsgValidator_ValidateMiddleParam(self->validator, paramStart, paramEnd))
	{
		return false;
	}

	char* param = StrUtils_CloneRange(paramStart, paramEnd);
	if (param == NULL)
	{
		LOG_ERROR(self->log, "Failed to allocate parameter.");
		return false;
	}

	self->msg->params[self->msg->paramCount] = param;
	self->msg->paramCount += 1;

	return true;
}

static bool IrcMsgParser_ParseTrailingParam(IrcMsgParser* self)
{
	const char* paramStart = self->rawMsg + 1;
	const char* paramEnd = strchr(paramStart, '\r');

	self->rawMsg = paramEnd;

	if (paramEnd == NULL)
	{
		LOG_WARN(self->log,
				"Invalid message: expected <CR> after <trailing>.");

		return false;
	}

	if (!IrcMsgValidator_ValidateTrailingParam(self->validator, paramStart, paramEnd))
	{
		return false;
	}

	char* param = StrUtils_CloneRange(paramStart, paramEnd);
	if (param == NULL)
	{
		LOG_ERROR(self->log, "Failed to allocate parameter.");
		return false;
	}

	self->msg->params[self->msg->paramCount] = param;
	self->msg->paramCount += 1;

	return true;
}

static bool IrcMsgParser_ParseParams(IrcMsgParser* self)
{
	IrcMsgParser_ParseSpace(self);

	while (*self->rawMsg != '\r')
	{
		if (self->msg->paramCount == IRC_MSG_MAX_PARAMS)
		{
			LOG_WARN(self->log, "Too many parameters. Expected at most: %zu",
					IRC_MSG_MAX_PARAMS);

			return false;
		}

		if (*self->rawMsg == ':')
		{
			if (!IrcMsgParser_ParseTrailingParam(self))
			{
				return false;
			}
			break;
		}

		if (!IrcMsgParser_ParseMiddleParam(self))
		{
			return false;
		}

		IrcMsgParser_ParseSpace(self);
	}

	return true;
}

static bool IrcMsgParser_ParseCRLF(IrcMsgParser* self)
{
	if (*self->rawMsg != '\r')
	{
		LOG_WARN(self->log, "Invalid Message: Expected CR to follow <params>");
		return false;
	}
	self->rawMsg += 1;

	if (*self->rawMsg != '\n')
	{
		LOG_WARN(self->log, "Invalid Message: Expected LF to follow CR");
		return false;
	}
	self->rawMsg += 1;

	return true;
}

static bool IrcMsgParser_ParseMessage(IrcMsgParser* self)
{
	if(!IrcMsgParser_ParsePrefix(self))
	{
		LOG_WARN(self->log, "Failed to parse <prefix>");
		return false;
	}

	IrcMsgParser_ParseSpace(self);

	if(!IrcMsgParser_ParseCommand(self))
	{
		LOG_WARN(self->log, "Failed to parse <command>");
		return false;
	}


	if(!IrcMsgParser_ParseParams(self))
	{
		LOG_WARN(self->log, "Failed to parse <params>");
		return false;
	}

	if(!IrcMsgParser_ParseCRLF(self))
	{
		LOG_WARN(self->log, "Failed to parse <crlf>");
		return false;
	}

	return true;
}

IrcMsg* IrcMsgParser_Parse(IrcMsgParser* self, const char* rawMsg)
{
	self->rawMsg = rawMsg;

	self->msg = malloc(sizeof(IrcMsg));
	if (self == NULL)
	{
		LOG_ERROR(self->log, "Failed to allocate IrcMsg");
		return NULL;
	}
	// Empty-initialize msg
	*self->msg = (IrcMsg) { 0 };

	if (!IrcMsgParser_ParseMessage(self))
	{
		LOG_WARN(self->log, "Failed to parse <message>");
		return NULL;
	}

	return self->msg;
}
