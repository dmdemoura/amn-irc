#include "irc_cmd_executor_task.h"

#include "array_list.h"

#include <stdlib.h>
#include <string.h>

typedef struct User
{
	int socket;
	char* nickname;
	char* username;
	char* hostname;
	char* servername;
	char* realname;
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
	free(user->servername);
	free(user->realname);
	free(user);
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

	ArrayList* users;
}
IrcCmdExecutorContext;

static IrcCmdExecutorContext* IrcCmdExecutorContext_New(
		const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds);
static void IrcCmdExecutorContext_Delete(void* context);

static void WaitForCmds(void* context);
static bool ExecuteCmd(IrcCmdExecutorContext* ctx, const IrcCmd* cmd);
static bool ExecuteCmdNick(
		IrcCmdExecutorContext* ctx, const int peerSocket, const IrcCmdNick* cmd);

Task* IrcCmdExecutorTask_New(const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds)
{
	IrcCmdExecutorContext* context = IrcCmdExecutorContext_New(log, tasks, cmds);
	if (context == NULL)
	{
		return NULL;
	}

	Task* self = Task_Create(WaitForCmds, context, IrcCmdExecutorContext_Delete);
	if (self == NULL)
	{
		free(context);
		return NULL;
	}

	return self;
}

static IrcCmdExecutorContext* IrcCmdExecutorContext_New(
		const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds)
{
	IrcCmdExecutorContext* ctx = malloc(sizeof(IrcCmdExecutorContext));
	if (ctx == NULL)
		return NULL;

	ctx->users = ArrayList_New(100, 100, sizeof(User), User_Delete);
	if (ctx->users == NULL)
	{
		free(ctx);
		return NULL;
	}

	ctx->log = log;
	ctx->tasks = tasks;
	ctx->cmds = cmds;

	return ctx;
}

static void IrcCmdExecutorContext_Delete(void* arg)
{
	if (arg == NULL)
	{
		return;
	}

	IrcCmdExecutorContext* ctx = (IrcCmdExecutorContext*) arg;

	ArrayList_Delete(ctx->users);
	free(ctx);
}

static void WaitForCmds(void* arg)
{
	IrcCmdExecutorContext* ctx = (IrcCmdExecutorContext*) arg;
	
	// TODO: Figure out a stop condition. Maybe an atomic bool?
	while(true)
	{
		IrcCmd* cmd = IrcCmdQueue_Pop(ctx->cmds);
		if (cmd == NULL)
		{
			LOG_ERROR(ctx->log, "Failed to get command from queue!");
			return;
		}

		bool result = ExecuteCmd(ctx, cmd);

		IrcCmd_Delete(cmd);

		if (!result)
		{
			LOG_ERROR(ctx->log, "Failed to execute command!");
			return;
		}
	}
}

static bool ExecuteCmd(IrcCmdExecutorContext* ctx, const IrcCmd* cmd)
{
	// TODO: Do stuff
	bool result = true;

	switch(cmd->type)
	{
		case IrcCmdType_Nick:
			result = ExecuteCmdNick(ctx, cmd->peerSocket, &cmd->nick);
		break;
		case IrcCmdType_User:

		break;
		default:
		break;
	}

	return result;
}

static bool ExecuteCmdNick(
		IrcCmdExecutorContext* ctx, const int peerSocket, const IrcCmdNick* cmd)
{
	User* existingUser = ArrayList_Find(ctx->users, User_CmpSocket, &peerSocket);

	if (existingUser == NULL)
	{
		// This is a new client.

		existingUser = ArrayList_Find(ctx->users, User_CmpNick, cmd->nickname);

		if (existingUser != NULL)
		{
			// Nick name conflict
			// TODO: Error reply and kill command.
			LOG_INFO(ctx->log, "Nickname collision: %s.", cmd->nickname);
			return true;
		}

		User newUser = {
			.socket = peerSocket,
			.nickname = cmd->nickname
		};

		if (!ArrayList_Append(ctx->users, &newUser))
		{
			return false;
		}

		LOG_INFO(ctx->log, "New client registered nickname: %s.", cmd->nickname);
	}
	else
	{
		// Nickname change
		// TODO: Nickname change.
		LOG_ERROR(ctx->log, "Nickname change is unimplemented!");
	}

	return true;
}
