#include "irc_cmd_unparser.h"
#include "irc_msg.h"
#include "str_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct IrcCmdUnparser
{
	const Logger* log;
	const IrcMsgValidator* validator;
};

static bool UnparseNick(IrcCmdUnparser* self, IrcMsg* msg, const IrcCmd* cmd);
static bool UnparseUser(IrcCmdUnparser* self, IrcMsg* msg, const IrcCmd* cmd);
static bool UnparsePrivMsg(IrcCmdUnparser* self, IrcMsg* msg, const IrcCmd* cmd);


IrcCmdUnparser* IrcCmdUnparser_New(const Logger* log, const IrcMsgValidator* validator)
{
	IrcCmdUnparser* self = malloc(sizeof(IrcCmdUnparser));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocate IrcCmdUnparser");
		return NULL;
	}

	self->log = log;
	self->validator = validator;

	return self;
}

void IrcCmdUnparser_Delete(IrcCmdUnparser* self)
{
	free(self);
}

IrcMsg* IrcCmdUnparser_Unparse(IrcCmdUnparser* self, const IrcCmd* cmd)
{
	IrcMsg* msg = malloc(sizeof(IrcMsg));
	if (msg == NULL)
	{
		LOG_ERROR(self->log, "Failed to allocate IrcMsg");
		return NULL;
	}

	msg->cmd = cmd->type;
	msg->paramCount = 0;
	if (!IrcMsgPrefix_Clone(&cmd->prefix, &msg->prefix))
	{
		LOG_ERROR(self->log, "Failed to clone message prefix");
		return false;
	}

	bool success = false;
	switch (cmd->type)
	{
	case IrcCmdType_Nick:
		success = UnparseNick(self, msg, cmd);
		break;
	case IrcCmdType_User:
		success = UnparseUser(self, msg, cmd);
		break;
	case IrcCmdType_PrivMsg:
		success = UnparsePrivMsg(self, msg, cmd);
		break;
	default:
		break;
	}

	if (!success)
	{
		IrcMsg_Delete(msg);
		return false;
	}

	return msg;
}

static bool UnparseNick(IrcCmdUnparser* self, IrcMsg* msg, const IrcCmd* cmd)
{
	msg->params[0] = cmd->nick.nickname;
	msg->paramCount = 1;

	if (cmd->nick.hopCount == 0)
	{
		// Local connection omit hop count.
		return true;
	}

	int len = snprintf(NULL, 0, "%zu", cmd->nick.hopCount);
	if (len < 0)
	{
		LOG_ERROR(self->log, "Failed to unparse hopcount: Format error");
		return false;
	}

	msg->params[1] = malloc(sizeof(char) * (size_t) (len + 1));
	if (msg->params[1] == NULL)
	{
		LOG_ERROR(self->log, "Failed to unparse hopcount: Allocation error");
		return false;
	}
	msg->paramCount = 2;

	int actualLen = snprintf(msg->params[1], (size_t) len + 1, "%zu", cmd->nick.hopCount);
	if (actualLen < 0 || actualLen > len)
	{
		LOG_ERROR(self->log, "Failed to unparse hopcount: Format error");
		return false;
	}

	return true;
}

static bool UnparseUser(IrcCmdUnparser* self, IrcMsg* msg, const IrcCmd* cmd)
{
	msg->params[0] = StrUtils_Clone(cmd->user.username);
	if (msg->params[0] == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone username");
		return false;
	}
	msg->paramCount++;

	msg->params[1] = StrUtils_Clone(cmd->user.hostname);
	if (msg->params[1] == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone hostname");
		return false;
	}
	msg->paramCount++;


	msg->params[2] = StrUtils_Clone(cmd->user.servername);
	if (msg->params[2] == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone servername");
		return false;
	}
	msg->paramCount++;

	msg->params[3] = StrUtils_Clone(cmd->user.realname);
	if (msg->params[3] == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone realname");
		return false;
	}
	msg->paramCount++;

	return true;
}

static bool UnparsePrivMsg(IrcCmdUnparser* self, IrcMsg* msg, const IrcCmd* cmd)
{
	size_t receiverLen = 0;
	for (size_t i = 0; i < cmd->privMsg.receiverCount; i++)
	{
		if (cmd->privMsg.receiver->value == NULL)
		{
			return false;
		}

		if (cmd->privMsg.receiver->type != IrcReceiverType_Nickname)
		{
			receiverLen++;
		}
		
		receiverLen += strlen(cmd->privMsg.receiver->value) + 1;
	}

	msg->params[0] = malloc(sizeof(char) * receiverLen);
	if (msg->params[0] == NULL)
	{
		return false;
	}
	msg->paramCount++;
	
	for (size_t i = 0, pos = 0; i < cmd->privMsg.receiverCount && pos < receiverLen - 1; i++)
	{
		switch (cmd->privMsg.receiver[i].type)
		{
			case IrcReceiverType_Nickname:
				break;
			case IrcReceiverType_LocalChannel:
				msg->params[0][pos] = '&';
				pos++;
				break;
			case IrcReceiverType_DistChannelOrHostMask:
				msg->params[0][pos] = '#';
				pos++;
				break;
			case IrcReceiverType_ServerMask:
				msg->params[0][pos] = '$';
				pos++;
				break;
		}

		size_t len = strlen(cmd->privMsg.receiver->value);
		memcpy(msg->params[0] + pos, cmd->privMsg.receiver->value, len);
		pos += len;
	}
	msg->params[0][receiverLen - 1] = '\0';
	
	msg->params[msg->paramCount] = StrUtils_Clone(cmd->privMsg.text);
	if (msg->params[msg->paramCount] == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone text");
		return false;
	}
	msg->paramCount++;

	return true;
}
