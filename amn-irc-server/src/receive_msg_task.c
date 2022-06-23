#include "receive_msg_task.h"

#include "irc_msg_reader.h"
#include "irc_msg_parser.h"
#include "irc_cmd_parser.h"

#include <stdlib.h>

#include <sys/socket.h>
#include <unistd.h>

typedef struct IrcCmdExecutorContext
{
	const Logger* log;
	TaskQueue* tasks;
	IrcCmdQueue* cmds;

	int socket;
	IrcMsgReader* reader;
	IrcMsgValidator* validator;
	IrcMsgParser* msgParser;
	IrcCmdParser* cmdParser;
}
ReceiveMsgContext;

static ReceiveMsgContext* ReceiveMsgContext_New(
		const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, int socket);
static void ReceiveMsgContext_Delete(void* context);
static void ReadMessages(void* context);

Task* ReceiveMsgTask_New(const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, int socket)
{
	ReceiveMsgContext* context = ReceiveMsgContext_New(log, tasks, cmds, socket);
	if (context == NULL)
	{
		return NULL;
	}

	Task* self = Task_Create(ReadMessages, context, ReceiveMsgContext_Delete);
	if (self == NULL)
	{
		free(context);
		return NULL;
	}

	return self;
}

static ReceiveMsgContext* ReceiveMsgContext_New(
		const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, int socket)
{
	ReceiveMsgContext* ctx = malloc(sizeof(ReceiveMsgContext));
	if (ctx == NULL)
		return NULL;

	*ctx = (ReceiveMsgContext) {0}; // Default initialize ctx.
	ctx->log = log;
	ctx->tasks = tasks;
	ctx->cmds = cmds;
	ctx->socket = socket;

	ctx->reader = IrcMsgReader_New(log, socket);
	if (ctx->reader == NULL)
		goto error;

	ctx->validator = IrcMsgValidator_New(ctx->log);
	if (ctx->validator == NULL)
		goto error;

	ctx->msgParser = IrcMsgParser_New(ctx->log, ctx->validator);
	if (ctx->msgParser == NULL)
		goto error;

	ctx->cmdParser = IrcCmdParser_New(ctx->log, ctx->validator);
	if (ctx->cmdParser == NULL)
		goto error;

	return ctx;
error:
	// These functions are all safe to call with null.
	IrcCmdParser_Delete(ctx->cmdParser);
	IrcMsgParser_Delete(ctx->msgParser);
	IrcMsgValidator_Delete(ctx->validator);
	IrcMsgReader_Delete(ctx->reader);
	free(ctx);
	// On failure the socket ownership return to the caller, so don't close it.
	return NULL;
}

static void ReceiveMsgContext_Delete(void* arg)
{
	if (arg == NULL)
	{
		return;
	}

	ReceiveMsgContext* ctx = (ReceiveMsgContext*) arg;

	IrcCmdParser_Delete(ctx->cmdParser);
	IrcMsgParser_Delete(ctx->msgParser);
	IrcMsgValidator_Delete(ctx->validator);
	IrcMsgReader_Delete(ctx->reader);

	if (close(ctx->socket) != 0)
	{
		LOG_ERROR(ctx->log, "Failed to close listen socket.");
	}

	free(ctx);
}

static void ReadMessages(void* arg)
{
	ReceiveMsgContext* ctx = (ReceiveMsgContext*) arg;
	
	while(true)
	{
		const char* rawMsg = IrcMsgReader_Read(ctx->reader);
		if (rawMsg == NULL)
		{
			LOG_ERROR(ctx->log, "Failed to read message.");
			return;
		}

		const IrcMsg* msg = IrcMsgParser_Parse(ctx->msgParser, rawMsg);
		if (msg == NULL)
		{
			LOG_WARN(ctx->log, "Failed to parse message.");
			continue;
		}

		LOG_DEBUG(ctx->log, "Parsed message:\n"
				"\tPrefix Origin: %s\n"
				"\tPrefix Username: %s\n"
				"\tPrefix Hostname: %s\n"
				"\tCommand: %d\n"
				"\tParams Count: %zu\n"
				"\tParams 01: %s\n"
				"\tParams 02: %s\n"
				"\tParams 03: %s\n"
				"\tParams 04: %s\n"
				"\tParams 05: %s\n",
				msg->prefix.origin != NULL ? msg->prefix.origin : "[NULL]",
				msg->prefix.username != NULL ? msg->prefix.username : "[NULL]",
				msg->prefix.hostname != NULL ? msg->prefix.hostname : "[NULL]",
				msg->cmd,
				msg->paramCount,
				msg->paramCount >= 1 ? msg->params[0] : "[EMPTY]",
				msg->paramCount >= 2 ? msg->params[1] : "[EMPTY]",
				msg->paramCount >= 3 ? msg->params[2] : "[EMPTY]",
				msg->paramCount >= 4 ? msg->params[3] : "[EMPTY]",
				msg->paramCount >= 5 ? msg->params[4] : "[EMPTY]");

		IrcCmd* cmd = IrcCmdParser_Parse(ctx->cmdParser, msg, ctx->socket);
		if (cmd == NULL)
		{
			LOG_WARN(ctx->log, "Failed to parse command.");
			continue;
		}

		if (!IrcCmdQueue_Push(ctx->cmds, cmd))
		{
			LOG_ERROR(ctx->log, "Failed to add command to queue");
			return;
		}
	}
}
