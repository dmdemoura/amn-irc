#include "irc_cmd_parser.h"

#include "irc_msg_validator.h"
#include "str_utils.h"

#include <stdlib.h>
#include <string.h>

struct IrcCmdParser
{
	const Logger* log;
	const IrcMsgValidator* validator;
};

static bool ParseNick(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg);
static bool ParseUser(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg);
static bool ParseQuit(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg);
static bool ParsePrivMsg(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg);

IrcCmdParser* IrcCmdParser_New(const Logger* log, const IrcMsgValidator* validator)
{
	IrcCmdParser* self = malloc(sizeof(IrcCmdParser));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocate IrcCmdParser");
		return NULL;
	}

	self->log = log;
	self->validator = validator;

	return self;
}

void IrcCmdParser_Delete(IrcCmdParser* self)
{
	free(self);
}

IrcCmd* IrcCmdParser_Parse(IrcCmdParser* self, const IrcMsg* msg, const int peerSocket)
{
	IrcCmd* cmd = malloc(sizeof(IrcCmd));
	if (cmd == NULL)
	{
		LOG_ERROR(self->log, "Failed to allocate IrcCmd.");
		return NULL;
	}

	*cmd = (IrcCmd) { 0 };

	cmd->type = msg->cmd;
	cmd->peerSocket = peerSocket;

	if (!IrcMsgPrefix_Clone(&msg->prefix, &cmd->prefix))
	{
		LOG_ERROR(self->log, "Failed to clone IrcMsgPrefix.");
		free(cmd);
		return NULL;
	}

	bool success = false;
	switch (cmd->type)
	{
	case IrcCmdType_Nick:
		success = ParseNick(self, cmd, msg);
		break;
	case IrcCmdType_User:
		success = ParseUser(self, cmd, msg);
		break;
	case IrcCmdType_Quit:
		success = ParseQuit(self, cmd, msg);
		break;
	case IrcCmdType_PrivMsg:
		success = ParsePrivMsg(self, cmd, msg);
		break;
	default:
		break;
	}

	if (!success)
	{
		IrcCmd_Delete(cmd);
		return NULL;
	}

	return cmd;
}

static bool ParseNick(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg)
{
	// Initialize everything to defaults in case we need to call Delete.
	cmd->nick = (IrcCmdNick) {0};

	if (msg->paramCount < 1 || msg->paramCount > 2)
	{
		LOG_WARN(self->log,
				"Got NICK cmd with unexpected parameter count: %zu. Expected: 1 or 2",
				msg->paramCount);
	}

	if (!IrcMsgValidator_ValidateNick(self->validator, msg->params[0], NULL))
	{
		LOG_WARN(self->log, "Got NICK cmd with invalid nickname: %s.", msg->params[0]);
		return false;
	}

	cmd->nick.nickname = StrUtils_Clone(msg->params[0]);
	if (cmd->nick.nickname == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone nickname string.");
		return false;
	}

	if (msg->paramCount == 1)
	{
		cmd->nick.hopCount = 0;
		return true;
	}

	if (!StrUtils_ReadSizeT(msg->params[1], &cmd->nick.hopCount))
	{
		LOG_WARN(self->log, "Got NICK cmd with invalid hopCount: %s.", msg->params[1]);
		return false;
	}

	return true;
}

static bool ParseUser(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg)
{
	// Initialize everything to defaults in case we need to call Delete.
	cmd->user = (IrcCmdUser) {0};

	if (msg->paramCount != 4)
	{
		LOG_WARN(self->log, "Got USER cmd with unexpected parameter count: %zu. Expected: 4",
				msg->paramCount);
	}

	if (!IrcMsgValidator_ValidateUser(self->validator, msg->params[0], NULL))
	{
		LOG_WARN(self->log, "Got USER cmd with invalid username: %s.", msg->params[0]);
		return false;
	}

	cmd->user.username = StrUtils_Clone(msg->params[0]);
	if (cmd->user.username == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone username string.");
		return false;
	}

	if (!IrcMsgValidator_ValidateHost(self->validator, msg->params[1], NULL))
	{
		LOG_WARN(self->log, "Got USER cmd with invalid hostname: %s.", msg->params[1]);
		return false;
	}

	cmd->user.hostname = StrUtils_Clone(msg->params[1]);
	if (cmd->user.hostname == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone hostname string.");
		return false;
	}

	if (!IrcMsgValidator_ValidateServer(self->validator, msg->params[2], NULL))
	{
		LOG_WARN(self->log, "Got USER cmd with invalid servername: %s.", msg->params[2]);
		return false;
	}

	cmd->user.servername = StrUtils_Clone(msg->params[2]);
	if (cmd->user.servername == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone servername string");
		return false;
	}

	cmd->user.realname = StrUtils_Clone(msg->params[3]);
	if (cmd->user.realname == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone realname string.");
		return false;
	}

	return true;
}

static bool ParseQuit(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg)
{
	// Initialize everything to defaults in case we need to call Delete.
	cmd->quit = (IrcCmdQuit) {0};

	if (msg->paramCount > 1)
	{
		LOG_WARN(self->log,
				"Got QUIT cmd with unexpected parameter count: %zu. Expected: 0 or 1",
				msg->paramCount);
	}

	if (msg->paramCount == 1)
	{
		cmd->quit.quitMessage = StrUtils_Clone(msg->params[0]);
		if (cmd->quit.quitMessage == NULL)
		{
			LOG_ERROR(self->log, "Failed to clone quit message.");
			return false;
		}
	}

	return true;
}

static bool ParsePrivMsg(IrcCmdParser* self, IrcCmd* cmd, const IrcMsg* msg)
{
	// Initialize everything to defaults in case we need to call Delete.
	cmd->privMsg = (IrcCmdPrivMsg) {0};

	if (msg->paramCount != 2)
	{
		LOG_WARN(self->log, "Got PRIVMSG cmd with %zu parameters. Expected: 2",
				msg->paramCount);
		return false;
	}
		
	size_t receiverCount = 1;
	for (char* c = msg->params[0]; *c != '\0'; c++)
	{
		if (*c == ',')
		{
			receiverCount++;
		}
	}

	cmd->privMsg.receiver = malloc(sizeof(IrcReceiver) * receiverCount);
	if (cmd->privMsg.receiver == NULL)
	{
		return false;
	}

	const char* receiver = msg->params[0];
	for (size_t i = 0; true; i++)
	{
		const char* receiverEnd = strchr(receiver, ',');
		receiverEnd = receiverEnd == NULL ? strchr(receiver, '\0') : receiverEnd;

		switch(*receiver)
		{
			case '&':
				cmd->privMsg.receiver[i].type = IrcReceiverType_LocalChannel;
				goto chstring;
			case '#':
				cmd->privMsg.receiver[i].type = IrcReceiverType_DistChannelOrHostMask;
				goto chstring;
			case '$':
				cmd->privMsg.receiver[i].type = IrcReceiverType_ServerMask;
				goto chstring;
			chstring:
				receiver++;

				if(!IrcMsgValidator_ValidateChstring(
							self->validator, receiver, receiverEnd))
				{
					LOG_WARN(self->log, "Got PRIVMSG cmd with invalid receiver[%zu]: %s.",
								i, receiver);
						return false;	
				}
				break;
			default:
				cmd->privMsg.receiver[i].type = IrcReceiverType_Nickname;

				if(!IrcMsgValidator_ValidateNick(self->validator, receiver, receiverEnd))
				{
					LOG_WARN(self->log, "Got PRIVMSG cmd with invalid receiver[%zu]: %s.",
								i, receiver);
						return false;	
				}
		}

		cmd->privMsg.receiver[i].value = StrUtils_CloneRange(receiver, receiverEnd);
		if (cmd->privMsg.receiver[i].value == NULL)
		{
			LOG_ERROR(self->log, "Failed to clone receiver[%zu] string", i);
			return false;
		}

		cmd->privMsg.receiverCount++; 
		if (*receiverEnd != '\0')
		{
			receiver = receiverEnd + 1;
		}
		else
		{
			break;
		}
	}

	cmd->privMsg.text = StrUtils_Clone(msg->params[msg->paramCount - 1]);
	if (cmd->privMsg.text == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone text string.");
		return false;
	}

	return true;
}
