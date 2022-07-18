#include "irc_cmd_executor_task.h"

#include "array_list.h"
#include "irc_reply.h"
#include "send_msg_task.h"
#include "irc_cmd_unparser.h"
#include "str_utils.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct User
{
	int socket;
	char* nickname;
	char* username;
	char* hostname;
	char* realname;
	bool isOperator;
} User;

static void User_Delete(void* arg)
{
	if (arg == NULL)
	{
		return;
	}

	User* user = (User*) arg;

	free(user->nickname);
	free(user->username);
	free(user->hostname);
	free(user->realname);
	// User struct is part of the arraylist storage
	// free(user);
}

static bool User_IsRegistered(const User* self)
{
	return self->nickname != NULL
		&& self->username != NULL
		&& self->hostname != NULL
		&& self->realname != NULL;
}

static bool User_CmpSocket(const void* user, const void* socket)
{
	return ((User*) user)->socket == *((int*) socket);
}

static bool User_CmpNick(const void* arg1, const void* arg2)
{
	User* user = (User*) arg1;
	char* nickname = (char*) arg2;

	if (user->nickname == NULL)
	{
		return false;
	}

	return strcmp(user->nickname, nickname) == 0;
}

typedef struct IrcCmdExecutorContext
{
	const Logger* log;
	TaskQueue* tasks;
	IrcCmdQueue* cmds;

	char* servername;
	ArrayList* users;
	IrcMsgValidator* msgValidator;
	IrcCmdUnparser* cmdUnparser;
}
IrcCmdExecutorContext;

typedef enum NickCollisionStatus
{
	NickCollisionStatus_None,
	NickCollisionStatus_Collision,
	NickCollisionStatus_Error
}
NickCollisionStatus;

static IrcCmdExecutorContext* IrcCmdExecutorContext_New(
		const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, const char* servername);

static void IrcCmdExecutorContext_Delete(void* context);

static TaskStatus WaitForCmds(void* context);

static bool ExecuteCmd(IrcCmdExecutorContext* ctx, IrcCmd* cmd);

static bool ExecuteCmdNick(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdNick* cmd);

static NickCollisionStatus ExecuteCmdNick_CheckAndHandleCollision(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdNick* cmd);

static bool ExecuteCmdUser(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdUser* cmd);

static bool ExecuteCmdPrivMsg(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdPrivMsg* cmd);

static bool ExecuteCmdQuit(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdQuit* cmd);

static bool Reply(IrcCmdExecutorContext* ctx, const int peerSocket, IrcMsg* msg);


Task* IrcCmdExecutorTask_New(const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds,
		const char* servername)
{
	IrcCmdExecutorContext* context = IrcCmdExecutorContext_New(log, tasks, cmds, servername);
	if (context == NULL)
	{
		LOG_ERROR(log, "Failed to create command executor context.");
		return NULL;
	}

	Task* self = Task_Create(WaitForCmds, context, IrcCmdExecutorContext_Delete);
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to create command executor task.");
		free(context);
		return NULL;
	}

	return self;
}

static IrcCmdExecutorContext* IrcCmdExecutorContext_New(
		const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, const char* servername)
{
	IrcCmdExecutorContext* ctx = malloc(sizeof(IrcCmdExecutorContext));
	if (ctx == NULL)
		return NULL;

	*ctx = (IrcCmdExecutorContext) {0};
	ctx->log = log;
	ctx->tasks = tasks;
	ctx->cmds = cmds;

	ctx->users = ArrayList_New(100, 100, sizeof(User), User_Delete);
	if (ctx->users == NULL)
	{
		LOG_ERROR(log, "Failed to create users list.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	ctx->msgValidator = IrcMsgValidator_New(log);
	if (ctx->msgValidator == NULL)
	{
		LOG_ERROR(log, "Failed to create message validator.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	ctx->cmdUnparser = IrcCmdUnparser_New(log, ctx->msgValidator);
	if (ctx->cmdUnparser == NULL)
	{
		LOG_ERROR(log, "Failed to create command unparser.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	if (!IrcMsgValidator_ValidateServer(ctx->msgValidator, servername, NULL))
	{
		LOG_ERROR(log, "Invalid servername.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	ctx->servername = StrUtils_Clone(servername);
	if (ctx->servername == NULL)
	{
		LOG_ERROR(log, "Failed to configure servername.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	return ctx;
}

static void IrcCmdExecutorContext_Delete(void* arg)
{
	if (arg == NULL)
	{
		return;
	}

	IrcCmdExecutorContext* ctx = (IrcCmdExecutorContext*) arg;

	free(ctx->servername);
	IrcCmdUnparser_Delete(ctx->cmdUnparser);
	IrcMsgValidator_Delete(ctx->msgValidator);
	ArrayList_Delete(ctx->users);
	free(ctx);
}

static TaskStatus WaitForCmds(void* arg)
{
	IrcCmdExecutorContext* ctx = (IrcCmdExecutorContext*) arg;
	
	IrcCmd* cmd = IrcCmdQueue_Pop(ctx->cmds);
	if (cmd == NULL && errno == EAGAIN)
	{
		return TaskStatus_Yield;
	}
	else if (cmd == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to get command from queue!");
		return TaskStatus_Failed;
	}

	bool result = ExecuteCmd(ctx, cmd);

	IrcCmd_Delete(cmd);

	if (!result)
	{
		LOG_ERROR(ctx->log, "Failed to execute command!");
		return TaskStatus_Failed;
	}

	return TaskStatus_Yield;
}

static bool ExecuteCmd(IrcCmdExecutorContext* ctx, IrcCmd* cmd)
{
	// TODO: Do stuff
	bool result = true;

	switch(cmd->type)
	{
		case IrcCmdType_Nick:
			result = ExecuteCmdNick(ctx, cmd->peerSocket, &cmd->nick);
		break;
		case IrcCmdType_User:
			result = ExecuteCmdUser(ctx, cmd->peerSocket, &cmd->user);
		break;
		case IrcCmdType_PrivMsg:
			result = ExecuteCmdPrivMsg(ctx, cmd->peerSocket, &cmd->privMsg);
		break;
		case IrcCmdType_Quit:
			result = ExecuteCmdQuit(ctx, cmd->peerSocket, &cmd->quit);
		default:
		break;
	}

	return result;
}

static bool ExecuteCmdNick(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdNick* cmd)
{
	User* existingUser = ArrayList_Find(ctx->users, User_CmpSocket, &peerSocket);

	if (existingUser != NULL && existingUser->nickname != NULL)
	{
		// Nickname change
		// TODO: Nickname change.
		LOG_ERROR(ctx->log, "Nickname change is unimplemented!");
		return true;
	}

	switch (ExecuteCmdNick_CheckAndHandleCollision(ctx, peerSocket, cmd))
	{
		case NickCollisionStatus_None:
			break;
		case NickCollisionStatus_Collision:
			return true;
		case NickCollisionStatus_Error:
			return false;
	}

	if (existingUser != NULL)
	{
		existingUser->nickname = cmd->nickname;
	}
	else
	{
		User newUser = {
			.socket = peerSocket,
			.nickname = cmd->nickname
		};

		if (!ArrayList_Append(ctx->users, &newUser))
		{
			return false;
		}
	}

	LOG_INFO(ctx->log, "New client registered nickname: %s.", cmd->nickname);

	// IrcCmd will not be used afterwards so we can steal the memory allocation
	cmd->nickname = NULL;

	return true;
}

static NickCollisionStatus ExecuteCmdNick_CheckAndHandleCollision(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdNick* cmd)
{
	User* userWithNick = ArrayList_Find(ctx->users, User_CmpNick, cmd->nickname);

	if (userWithNick == NULL)
	{
		return NickCollisionStatus_None;
	}

	LOG_INFO(ctx->log, "Nickname collision: %s.", cmd->nickname);

	IrcMsg* reply = IrcReply_ErrNickCollision(ctx->servername, cmd->nickname);
	if (reply == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to generate reply");
		return NickCollisionStatus_Error;
	}

	if (!Reply(ctx, peerSocket, reply))
	{
		return NickCollisionStatus_Error;
	}

	return NickCollisionStatus_Collision;
}

static bool ExecuteCmdUser(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdUser* cmd)
{
	User* existingUser = ArrayList_Find(ctx->users, User_CmpSocket, &peerSocket);

	if (existingUser != NULL && User_IsRegistered(existingUser))
	{
		IrcMsg* reply = IrcReply_ErrAlreadyRegistered(ctx->servername);
		if (reply == NULL)
		{
			LOG_ERROR(ctx->log, "Failed to generate reply");
			return false;
		}

		if (!Reply(ctx, peerSocket, reply))
		{
			return false;
		}

		return true;
	}

	if (existingUser != NULL)
	{
		existingUser->username = cmd->username;
		existingUser->hostname = cmd->hostname;
		existingUser->realname = cmd->realname;
	}
	else
	{
		User newUser = {
			.socket = peerSocket,
			.username = cmd->username,
			.hostname = cmd->hostname,
			.realname = cmd->realname,
		};

		if (!ArrayList_Append(ctx->users, &newUser))
		{
			return false;
		}
	}

	LOG_INFO(ctx->log, "New client registered:\n"
			"\tusername:\t%s\n"
			"\thostname:\t%s\n"
			"\tservername:\t%s\n"
			"\trealname:\t%s\n",
			cmd->username, cmd->hostname, ctx->servername, cmd->realname);

	// IrcCmd will not be used afterwards so we can steal the memory allocations
	cmd->username = NULL;
	cmd->hostname = NULL;
	cmd->realname = NULL;

	return true;
}

static bool ExecuteCmdPrivMsg(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdPrivMsg* cmd)
{
	User* user = ArrayList_Find(ctx->users, User_CmpSocket, &peerSocket);

	if (user == NULL || !User_IsRegistered(user))
	{
		IrcMsg* reply = IrcReply_ErrNotRegistered(ctx->servername);
		if (reply == NULL)
		{
			LOG_ERROR(ctx->log, "Failed to generate reply");
			return false;
		}

		if (!Reply(ctx, peerSocket, reply))
		{
			return false;
		}

		return true;
	}

	IrcCmd cmdToSend = {
		.prefix = {
			.origin = user->nickname,
			.username = user->username,
			.hostname = user->hostname,
		},
		.type = IrcCmdType_PrivMsg,
		.privMsg = *cmd,
	};

	IrcMsg* msgToSend = IrcCmdUnparser_Unparse(ctx->cmdUnparser, &cmdToSend);
	if (msgToSend == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to unparse PRIVMSG");
		return false;
	}

	for (size_t i = 0; i < cmd->receiverCount; i++)
	{
		switch (cmd->receiver[i].type)
		{
			case IrcReceiverType_Nickname:
			{
				User* userWithNick = ArrayList_Find(
						ctx->users, User_CmpNick, cmd->receiver[i].value);

				if (userWithNick == NULL)
				{
					IrcMsg* reply = IrcReply_ErrNoSuchNick(
							ctx->servername, cmd->receiver[i].value);
					if (reply == NULL)
					{
						LOG_ERROR(ctx->log, "Failed to generate reply");
						return false;
					}

					if (!Reply(ctx, peerSocket, reply))
					{
						return false;
					}

					return true;
				}

				if (!Reply(ctx, userWithNick->socket, msgToSend))
				{
					return false;
				}
			}
			break;
			case IrcReceiverType_LocalChannel:
			case IrcReceiverType_DistChannelOrHostMask:
			case IrcReceiverType_ServerMask:
				LOG_ERROR(ctx->log, "Receiver type not implemented");
				break;
		}
	}
	return true;
}

static bool ExecuteCmdQuit(
		IrcCmdExecutorContext* ctx, const int peerSocket, IrcCmdQuit* cmd)
{
	(void) cmd;

	ArrayList_Remove(ctx->users, User_CmpSocket, &peerSocket);

	LOG_INFO(ctx->log, "Client unregistered.");

	// TODO: Figure out how to close socket
	return true;
}

static bool Reply(IrcCmdExecutorContext* ctx, const int peerSocket, IrcMsg* msg)
{
	Task* sendMsgTask = SendMsgTask_New(ctx->log, peerSocket, msg);
	if (sendMsgTask == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to create send msg task!");
		return false;
	}

	if (!TaskQueue_Push(ctx->tasks, sendMsgTask))
	{
		LOG_ERROR(ctx->log, "Failed to push send message task onto queue");
		return false;
	}

	return true;
}
