#include "irc_cmd.h"

#include "str_utils.h"

#include <stdlib.h>

static bool IrcCmd_CloneNick(const IrcCmd* self, IrcCmd* clone);
static bool IrcCmd_CloneUser(const IrcCmd* self, IrcCmd* clone);
static bool IrcCmd_CloneJoin(const IrcCmd* self, IrcCmd* clone);
static bool IrcCmd_CloneQuit(const IrcCmd* self, IrcCmd* clone);
static bool IrcCmd_ClonePrivMsg(const IrcCmd* self, IrcCmd* clone);

static void IrcCmd_DeleteNick(IrcCmd* self);
static void IrcCmd_DeleteUser(IrcCmd* self);
static void IrcCmd_DeleteJoin(IrcCmd* self);
static void IrcCmd_DeleteQuit(IrcCmd* self);
static void IrcCmd_DeletePrivMsg(IrcCmd* self);

IrcCmd* IrcCmd_Clone(const IrcCmd* self)
{
	IrcCmd* clone = malloc(sizeof(IrcCmd));
	if (clone == NULL)
	{
		return NULL;
	}

	*clone = (IrcCmd) {
		.type = self->type,
		.peerSocket = self->peerSocket,
	};

	if (!IrcMsgPrefix_Clone(&self->prefix, &clone->prefix))
	{
		IrcCmd_Delete(clone);
		return NULL;
	}

	bool success = true;
	switch (clone->type)
	{
		case IrcCmdType_Nick:
			success = IrcCmd_CloneNick(self, clone);
			break;
		case IrcCmdType_User:
			success = IrcCmd_CloneUser(self, clone);
			break;
		case IrcCmdType_Quit:
			success = IrcCmd_CloneQuit(self, clone);
			break;
		case IrcCmdType_Join:
			success = IrcCmd_CloneJoin(self, clone);
			break;
		case IrcCmdType_PrivMsg:
			success = IrcCmd_ClonePrivMsg(self, clone);
			break;
		default:
			return NULL;
	}

	if (!success)
	{
		IrcCmd_Delete(clone);
		return NULL;
	}

	return clone;
}

static bool IrcCmd_CloneNick(const IrcCmd* self, IrcCmd* clone)
{
	clone->nick.nickname = StrUtils_Clone(self->nick.nickname);
	if (clone->nick.nickname == NULL)
	{
		return false;
	}

	clone->nick.hopCount = self->nick.hopCount;

	return true;
}

static bool IrcCmd_CloneUser(const IrcCmd* self, IrcCmd* clone)
{
	clone->user.username = StrUtils_Clone(self->user.username);
	if (clone->user.username == NULL)
	{
		return false;
	}

	clone->user.hostname = StrUtils_Clone(self->user.hostname);
	if (clone->user.hostname == NULL)
	{
		return false;
	}

	clone->user.servername = StrUtils_Clone(self->user.servername);
	if (clone->user.servername == NULL)
	{
		return false;
	}

	clone->user.realname = StrUtils_Clone(self->user.realname);
	if (clone->user.realname == NULL)
	{
		return false;
	}

	return true;
}

static bool IrcCmd_CloneJoin(const IrcCmd* self, IrcCmd* clone)
{
	clone->join.channels = malloc(sizeof(IrcReceiver) * self->join.channelCount);
	if (clone->join.channels == NULL)
	{
		return false;
	}

	for (size_t i = 0; i < self->join.channelCount; i++)
	{
		clone->join.channels[i].name = StrUtils_Clone(self->join.channels[i].name);
		if (clone->join.channels[i].name == NULL)
		{
			return false;
		}

		clone->join.channels[i].key = StrUtils_Clone(self->join.channels[i].key);
		if (clone->join.channels[i].key == NULL)
		{
			return false;
		}

		clone->join.channelCount++;
	}

	return true;
}

static bool IrcCmd_CloneQuit(const IrcCmd* self, IrcCmd* clone)
{
	clone->quit.quitMessage = StrUtils_Clone(self->quit.quitMessage);
	if (clone->quit.quitMessage == NULL)
	{
		return false;
	}

	return true;
}

static bool IrcCmd_ClonePrivMsg(const IrcCmd* self, IrcCmd* clone)
{
	clone->privMsg.receiver = malloc(sizeof(IrcReceiver) * self->privMsg.receiverCount);
	if (clone->privMsg.receiver == NULL)
	{
		return false;
	}

	for (size_t i = 0; i < self->privMsg.receiverCount; i++)
	{
		clone->privMsg.receiver[i].value = StrUtils_Clone(self->privMsg.receiver[i].value);
		if (clone->privMsg.receiver[i].value == NULL)
		{
			return false;
		}

		clone->privMsg.receiverCount++;
	}

	clone->privMsg.text = StrUtils_Clone(self->privMsg.text);
	if (clone->privMsg.text == NULL)
	{
		return false;
	}

	return true;
}

void IrcCmd_Delete(IrcCmd* self)
{
	if (self == NULL)
	{
		return;
	}

	switch (self->type)
	{
		case IrcCmdType_Nick:
			IrcCmd_DeleteNick(self);
			break;
		case IrcCmdType_User:
			IrcCmd_DeleteUser(self);
			break;
		case IrcCmdType_Join:
			IrcCmd_DeleteJoin(self);
			break;
		case IrcCmdType_Quit:
			IrcCmd_DeleteQuit(self);
			break;
		case IrcCmdType_PrivMsg:
			IrcCmd_DeletePrivMsg(self);
			break;
		default:
			break;
	}

	free(self->prefix.origin);
	free(self->prefix.username);
	free(self->prefix.hostname);
	free(self);
}

static void IrcCmd_DeleteNick(IrcCmd* self)
{
	free(self->nick.nickname);
}

static void IrcCmd_DeleteUser(IrcCmd* self)
{
	free(self->user.username);
	free(self->user.hostname);
	free(self->user.servername);
	free(self->user.realname);
}

static void IrcCmd_DeleteJoin(IrcCmd* self)
{
	for (size_t i = 0; i < self->join.channelCount; i++)
	{
		free(self->join.channels[i].name);
		free(self->join.channels[i].key);
	}

	free(self->join.channels);
}

static void IrcCmd_DeleteQuit(IrcCmd* self)
{
	free(self->quit.quitMessage);
}

static void IrcCmd_DeletePrivMsg(IrcCmd* self)
{
	for (size_t i = 0; i < self->privMsg.receiverCount; i++)
	{
		free(self->privMsg.receiver[i].value);
	}
	free(self->privMsg.receiver);
	free(self->privMsg.text);
}
