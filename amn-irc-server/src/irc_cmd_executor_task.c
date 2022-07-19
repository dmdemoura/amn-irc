#include "irc_cmd_executor_task.h"

#include "array_list.h"
#include "irc_cmd.h"
#include "irc_reply.h"
#include "send_msg_task.h"
#include "irc_cmd_unparser.h"
#include "str_utils.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

// Unsigned integers overflow nicely so even if you manage to have over
// 18,446,744,073,709,551,616 users across the timespan the server is running
// you should not have any problems unless you also have someone logged in
// for that same amount of time.
typedef uint64_t UserId;

static bool UserId_Cmp(const void* self, const void* other)
{
	return *((UserId*)self) == *((UserId*) other);
}

typedef struct User
{
	UserId id;
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

// static bool User_CmpId(const void* user, const void* id)
// {
// 	return ((User*) user)->id == *((UserId*) id);
// }

static bool User_CmpNick(const void* arg1, const void* arg2)
{
	User* user = (User*) arg1;
	char* nickname = (char*) arg2;

	if (user->nickname == NULL)
	{
		return false;
	}

	return StrUtils_Equals(user->nickname, nickname);
}

typedef struct Channel
{
	char* name;

	IrcModes modes;
	ArrayList* operatorIds;
	size_t limit;
	char* banmask;
	char* key;

	ArrayList* memberIds;
	char* topic;
}
Channel;

static void Channel_Delete(void* arg)
{
	if (arg == NULL)
	{
		return;
	}
	
	Channel* channel = (Channel*) arg;

	free(channel->name);
	ArrayList_Delete(channel->operatorIds);
	free(channel->banmask);
	free(channel->key);
	ArrayList_Delete(channel->memberIds);
	free(channel->topic);
	// Channel struct is part of the arraylist storage
	// free(channel);
}

static bool Channel_CmpName(const void* arg1, const void* arg2)
{
	Channel* channel = (Channel*) arg1;
	char* name = (char*) arg2;

	return StrUtils_Equals(channel->name, name);
}

typedef struct IrcCmdExecutorContext
{
	// Non-Owned objects
	const Logger* log;
	TaskQueue* tasks;
	IrcCmdQueue* cmds;

	// Owned objects
	char* servername;
	// There are better data structures for this but this is C
	// and I don't have time to implement a entire hashmap.
	ArrayList* users;
	ArrayList* localChannels;
	ArrayList* distChannels;
	IrcMsgValidator* msgValidator;
	IrcCmdUnparser* cmdUnparser;
	UserId nextUserId;

	// Execution scoped fields:
	
	// Command processed successfully or failed.
	// Only unexpected errors are considered failures.
	// Bad user input correctly handled is still a success.
	bool success;
	// Replies to be sent after processing this command
	ArrayList* replyBuf;
	// Commands to be sent after processing this command
	ArrayList* cmdBuf;
}
IrcCmdExecutorContext;

static IrcCmdExecutorContext* IrcCmdExecutorContext_New(
		const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, const char* servername);

static void IrcCmdExecutorContext_Delete(void* context);

static TaskStatus WaitForCmds(void* context);

static void ExecuteCmd(IrcCmdExecutorContext* ctx, IrcCmd* cmd);

static void ExecuteCmdNick(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdNick* cmd);

static bool ExecuteCmdNick_CheckCollision(IrcCmdExecutorContext* ctx, IrcCmdNick* cmd);

static void ExecuteCmdUser(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdUser* cmd);

static void ExecuteCmdQuit(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdQuit* cmd);

static void ExecuteCmdJoin(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdJoin* cmd);

static void ExecuteCmdJoin_CreateChannel(
		IrcCmdExecutorContext* ctx, ArrayList* channels,
		User* user, IrcChannelAndKey* channelAndKey);

static void ExecuteCmdJoin_JoinChannel(
		IrcCmdExecutorContext* ctx,
		User* user,
		Channel* channel,
		IrcChannelType channelType,
		char* key);

static void ExecuteCmdPrivMsg(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdPrivMsg* cmd);


static User* WithRegisteredUser(IrcCmdExecutorContext* ctx, int peerSocket);
static ArrayList* ChannelList(IrcCmdExecutorContext* ctx, IrcChannelType type);

static void AddReply(IrcCmdExecutorContext* ctx, IrcMsg* msg);
static void AddCmd(IrcCmdExecutorContext* ctx, const IrcCmd* cmd);

static void SendReplies(IrcCmdExecutorContext* ctx, int peerSocket);
static void SendCmds(IrcCmdExecutorContext* ctx);
static void SendMsg(IrcCmdExecutorContext* ctx, int peerSocket, IrcMsg* msg);


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

static void IrcMsgPtrDelete(void* arg)
{
	IrcMsg* msg = *(IrcMsg**) arg;

	IrcMsg_Delete(msg);
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
	ctx->nextUserId = 0;
	ctx->success = true;

	ctx->users = ArrayList_New(100, 100, sizeof(User), User_Delete);
	if (ctx->users == NULL)
	{
		LOG_ERROR(log, "Failed to create users list.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	ctx->localChannels = ArrayList_New(50, 50, sizeof(Channel), Channel_Delete);
	if (ctx->localChannels == NULL)
	{
		LOG_ERROR(log, "Failed to create local channels list.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	ctx->distChannels = ArrayList_New(100, 100, sizeof(Channel), Channel_Delete);
	if (ctx->distChannels == NULL)
	{
		LOG_ERROR(log, "Failed to create distributed channels list.");
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

	ctx->replyBuf = ArrayList_New(100, 100, sizeof(IrcMsg*), IrcMsgPtrDelete);
	if (ctx->replyBuf == NULL)
	{
		LOG_ERROR(log, "Failed to create reply buffer.");
		IrcCmdExecutorContext_Delete(ctx);
		return NULL;
	}

	ctx->cmdBuf = ArrayList_New(100, 100, sizeof(IrcCmd), NULL);
	if (ctx->cmdBuf == NULL)
	{
		LOG_ERROR(log, "Failed to create cmd buffer.");
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
	ArrayList_Delete(ctx->localChannels);
	ArrayList_Delete(ctx->distChannels);
	ArrayList_Delete(ctx->replyBuf);
	ArrayList_Delete(ctx->cmdBuf);
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

	ctx->success = true;

	ExecuteCmd(ctx, cmd);
	SendReplies(ctx, cmd->peerSocket);
	SendCmds(ctx);

	IrcCmd_Delete(cmd);
	ArrayList_Clear(ctx->replyBuf);
	ArrayList_Clear(ctx->cmdBuf);

	if (!ctx->success)
	{
		LOG_ERROR(ctx->log, "Failed to execute command!");
		return TaskStatus_Failed;
	}

	return TaskStatus_Yield;
}

static void ExecuteCmd(IrcCmdExecutorContext* ctx, IrcCmd* cmd)
{
	switch(cmd->type)
	{
		case IrcCmdType_Nick:
			ExecuteCmdNick(ctx, cmd->peerSocket, &cmd->nick);
		break;
		case IrcCmdType_User:
			ExecuteCmdUser(ctx, cmd->peerSocket, &cmd->user);
		break;
		case IrcCmdType_Quit:
			ExecuteCmdQuit(ctx, cmd->peerSocket, &cmd->quit);
		break;
		case IrcCmdType_Join:
			ExecuteCmdJoin(ctx, cmd->peerSocket, &cmd->join);
		break;
		case IrcCmdType_PrivMsg:
			ExecuteCmdPrivMsg(ctx, cmd->peerSocket, &cmd->privMsg);
		break;
		default:
		break;
	}
}

static void ExecuteCmdNick(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdNick* cmd)
{
	User* existingUser = ArrayList_Find(ctx->users, User_CmpSocket, &peerSocket);

	if (existingUser != NULL && existingUser->nickname != NULL)
	{
		// Nickname change
		// TODO: Nickname change.
		LOG_ERROR(ctx->log, "Nickname change is unimplemented!");
		return;
	}

	if (ExecuteCmdNick_CheckCollision(ctx, cmd))
	{
		return;
	}

	if (existingUser != NULL)
	{
		existingUser->nickname = cmd->nickname;
	}
	else
	{
		User newUser = {
			.id = ctx->nextUserId++,
			.socket = peerSocket,
			.nickname = cmd->nickname
		};

		if (!ArrayList_Append(ctx->users, &newUser))
		{
			ctx->success = false;
			return;
		}
	}

	LOG_INFO(ctx->log, "New client registered nickname: %s.", cmd->nickname);

	// IrcCmd will not be used afterwards so we can steal the memory allocation
	cmd->nickname = NULL;
}

static bool ExecuteCmdNick_CheckCollision(IrcCmdExecutorContext* ctx, IrcCmdNick* cmd)
{
	User* userWithNick = ArrayList_Find(ctx->users, User_CmpNick, cmd->nickname);

	if (userWithNick == NULL)
	{
		return false;
	}

	LOG_INFO(ctx->log, "Nickname collision: %s.", cmd->nickname);
	AddReply(ctx, IrcReply_ErrNickCollision(ctx->servername, cmd->nickname));

	return true;
}

static void ExecuteCmdUser(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdUser* cmd)
{
	User* existingUser = ArrayList_Find(ctx->users, User_CmpSocket, &peerSocket);

	if (existingUser != NULL && User_IsRegistered(existingUser))
	{
		AddReply(ctx, IrcReply_ErrAlreadyRegistered(ctx->servername));
		return;
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
			.id = ctx->nextUserId++,
			.socket = peerSocket,
			.username = cmd->username,
			.hostname = cmd->hostname,
			.realname = cmd->realname,
		};

		if (!ArrayList_Append(ctx->users, &newUser))
		{
			ctx->success = false;
			return;
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
}

static void ExecuteCmdPrivMsg(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdPrivMsg* cmd)
{
	User* user = WithRegisteredUser(ctx, peerSocket);
	if (user == NULL)
	{
		return;
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
		ctx->success = false;
		return;
	}

	for (size_t i = 0; i < cmd->receiverCount; i++)
	{
		switch (cmd->receiver[i].type)
		{
			case IrcReceiverType_Nickname:
			{
				User* userWithNick = ArrayList_Find(
						ctx->users, User_CmpNick, cmd->receiver[i].value);

				if (userWithNick == NULL || !User_IsRegistered(userWithNick))
				{
					AddReply(ctx, IrcReply_ErrNoSuchNick(
								ctx->servername, cmd->receiver[i].value));
					if (!ctx->success)
					{
						return;
					}

					continue;
				}

				cmdToSend.peerSocket = userWithNick->socket;
				AddCmd(ctx, &cmdToSend);
				if (!ctx->success)
				{
					return;
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
}

static void ExecuteCmdJoin(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdJoin* cmd)
{
	LOG_DEBUG(ctx->log, "Got JOIN command");
	User* user = WithRegisteredUser(ctx, peerSocket);
	if (user == NULL)
	{
		return;
	}

	for (size_t i = 0; ctx->success && i < cmd->channelCount; i++)
	{
		ArrayList* channels = ChannelList(ctx, cmd->channels[i].type); 
		Channel* channel = ArrayList_Find(channels, Channel_CmpName, cmd->channels[i].name);

		if (channel != NULL)
		{
			ExecuteCmdJoin_JoinChannel(ctx, user, channel, cmd->channels[i].type,
					cmd->channels[i].key);
		}
		else
		{
			ExecuteCmdJoin_CreateChannel(ctx, channels, user, &cmd->channels[i]);
		}
	}
}

static void ExecuteCmdJoin_CreateChannel(
		IrcCmdExecutorContext* ctx,
		ArrayList* channels,
		User* user,
		IrcChannelAndKey* channelAndKey)
{
	Channel channel = {
		.name = channelAndKey->name,
		.operatorIds = ArrayList_New(10, 10, sizeof(UserId), NULL),
		.modes = channelAndKey->key != NULL ? IrcMode_Channel_RequiresKey : IrcMode_None,
		.limit = SIZE_MAX,
		.banmask = NULL,
		.key = channelAndKey->key,
		.memberIds = ArrayList_New(100, 100, sizeof(UserId), NULL),
		.topic = NULL,
	};

	if (channel.operatorIds == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to create channel operator id list.");
		goto error;
	}

	if (channel.memberIds == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to create channel member id list.");
		goto error;
	}

	if (!ArrayList_Append(channel.operatorIds, &user->id))
	{
		LOG_ERROR(ctx->log, "Failed to add user id to operator list.");
		goto error;
	}

	if (!ArrayList_Append(channel.memberIds, &user->id))
	{
		LOG_ERROR(ctx->log, "Failed to add user id to member list.");
		goto error;
	}

	if (!ArrayList_Append(channels, &channel))
	{
		LOG_ERROR(ctx->log, "Failed to add channel to list.");
		goto error;
	}


	// Steal allocations.
	channelAndKey->name = NULL;
	channelAndKey->key = NULL;

	LOG_DEBUG(ctx->log, "Created channel: %s", channel.name);
	return;

error:
	ctx->success = false;
	Channel_Delete(&channel);
}

static void ExecuteCmdJoin_JoinChannel(
		IrcCmdExecutorContext* ctx,
		User* user,
		Channel* channel,
		IrcChannelType channelType,
		char* key)
{
	if (ArrayList_Find(channel->memberIds, UserId_Cmp, &user->id) != NULL)
	{
		LOG_DEBUG(ctx->log, "User already in channel: %s.", channel->name);
		return;
	}

	if (channel->modes & IrcMode_Channel_InviteOnly)
	{
		AddReply(ctx, IrcReply_ErrInviteOnlyChan(
					ctx->servername, channelType, channel->name)); 
		return;
	}

	if (channel->modes & IrcMode_Channel_RequiresKey
			&& !StrUtils_Equals(channel->key, key))
	{
		AddReply(ctx, IrcReply_ErrBadChannelKey(ctx->servername, channelType, channel->name)); 
		return;
	}

	if (channel->modes & IrcMode_Channel_LimitedUsers
			&& ArrayList_Size(channel->memberIds) >= channel->limit)
	{
		AddReply(ctx, IrcReply_ErrChannelIsFull(
					ctx->servername, channelType, channel->name)); 
		return;
	}

	// TODO: Validate banmask!

	if (!ArrayList_Append(channel->memberIds, &user->id))
	{
		LOG_ERROR(ctx->log, "Failed to add user id to member list.");
		ctx->success = false;
		return;
	}

	LOG_DEBUG(ctx->log, "Joined channel: %s.", channel->name);

	if (channel->topic == NULL)
	{
		return;
	}

	AddReply(ctx, IrcReply_RplTopic(
				ctx->servername, channelType, channel->name, channel->topic));
}


static void ExecuteCmdQuit(
		IrcCmdExecutorContext* ctx, int peerSocket, IrcCmdQuit* cmd)
{
	if (ArrayList_Remove(ctx->users, User_CmpSocket, &peerSocket, true))
	{
		LOG_DEBUG(ctx->log, "Client unregistered: %s", cmd->quitMessage);
	}
}


static User* WithRegisteredUser(IrcCmdExecutorContext* ctx, int peerSocket)
{
	User* user = ArrayList_Find(ctx->users, User_CmpSocket, &peerSocket);

	if (user == NULL || !User_IsRegistered(user))
	{
		AddReply(ctx, IrcReply_ErrNotRegistered(ctx->servername));
		return NULL;
	}

	return user;
}

static ArrayList* ChannelList(IrcCmdExecutorContext* ctx, IrcChannelType type)
{
	switch (type)
	{
		case IrcChannelType_Local:
			return ctx->localChannels;
		case IrcChannelType_Distributed:
			return ctx->distChannels;
		default:
			LOG_ERROR(ctx->log, "Unknown channel type");
			return NULL;
	}
}

static void AddReply(IrcCmdExecutorContext* ctx, IrcMsg* reply)
{
	if (reply == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to generate reply");
		ctx->success = false;
		return;
	}

	if (!ArrayList_Append(ctx->replyBuf, &reply))
	{
		LOG_ERROR(ctx->log, "Failed to append to replyBuf");
		ctx->success = false;
	}
}

static void AddCmd(IrcCmdExecutorContext* ctx, const IrcCmd* cmd)
{
	if (!ArrayList_Append(ctx->cmdBuf, cmd))
	{
		LOG_ERROR(ctx->log, "Failed to append to cmdBuf.");
		ctx->success = false;
	}
}

static void SendReplies(IrcCmdExecutorContext* ctx, int peerSocket)
{
	for (size_t i = ArrayList_Size(ctx->replyBuf); ctx->success && i > 0; i--)
	{
		IrcMsg* reply = *(IrcMsg**) ArrayList_Get(ctx->replyBuf, i - 1);
		if (!ArrayList_RemoveIndex(ctx->replyBuf, i - 1, false))
		{
			LOG_ERROR(ctx->log, "Failed to remove from reply buffer.");
			ctx->success = false;
			return;
		}

		SendMsg(ctx, peerSocket, reply);
	}
}

static void SendCmds(IrcCmdExecutorContext* ctx)
{
	for (size_t i = 0; ctx->success && i < ArrayList_Size(ctx->cmdBuf); i++)
	{
		IrcCmd* cmd = ArrayList_Get(ctx->cmdBuf, i);

		IrcMsg* msg = IrcCmdUnparser_Unparse(ctx->cmdUnparser, cmd);
		if (msg == NULL)
		{
			LOG_ERROR(ctx->log, "Failed to unparse command.");
			ctx->success = false;
			return;	
		}

		SendMsg(ctx, cmd->peerSocket, msg);
	}
}

static void SendMsg(IrcCmdExecutorContext* ctx, int peerSocket, IrcMsg* msg)
{
	Task* sendMsgTask = SendMsgTask_New(ctx->log, peerSocket, msg);

	if (sendMsgTask == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to create send msg task!");
		ctx->success = false;
		return;
	}

	if (!TaskQueue_Push(ctx->tasks, sendMsgTask))
	{
		LOG_ERROR(ctx->log, "Failed to push send message task onto queue");
		ctx->success = false;
	}
}
