#include "irc_cmd.h"

#include <stdlib.h>

static void IrcCmd_DeleteNick(IrcCmd* self);
static void IrcCmd_DeleteUser(IrcCmd* self);
static void IrcCmd_DeletePrivMsg(IrcCmd* self);

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
		case IrcCmdType_PrivMsg:
			IrcCmd_DeletePrivMsg(self);
			break;
		default:
			break;
	}

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

static void IrcCmd_DeletePrivMsg(IrcCmd* self)
{
	for (size_t i = 0; i < self->privMsg.receiverCount; i++)
	{
		free(self->privMsg.receiver[i]);
	}
	free(self->privMsg.text);
}
