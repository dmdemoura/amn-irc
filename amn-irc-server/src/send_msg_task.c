#include "send_msg_task.h"

#include "irc_msg_unparser.h"
#include "irc_msg_writer.h"

#include <stdlib.h>

typedef struct SendMsgContext
{
	const Logger* log;
	IrcMsg* msg;

	IrcMsgUnparser* unparser;
	IrcMsgWriter* writer;
}
SendMsgContext;

static SendMsgContext* SendMsgContext_New(const Logger* log, int socket, const IrcMsg* msg);
static void SendMsgContext_Delete(void* context);
static TaskStatus SendMessages(void* context);

Task* SendMsgTask_New(const Logger* log, int socket, const IrcMsg* msg)
{
	SendMsgContext* ctx = SendMsgContext_New(log, socket, msg);
	if (ctx == NULL)
	{
		LOG_ERROR(log, "Failed to create SendMsgContext.");
		return NULL;
	}

	Task* self = Task_Create(SendMessages, ctx, SendMsgContext_Delete);
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to create SendMsgTask.");
		free(ctx);
		return NULL;
	}

	return self;
}

static SendMsgContext* SendMsgContext_New(const Logger* log, int socket, const IrcMsg* msg)
{
	SendMsgContext* ctx = malloc(sizeof(SendMsgContext));
	if (ctx == NULL)
	{
		LOG_ERROR(log, "Failed to allocate SendMsgContext.");
		return NULL;
	}

	ctx->log = log;
	ctx->msg = IrcMsg_Clone(msg);
	if (ctx->msg == NULL)
	{
		LOG_ERROR(log, "Failed to clone IrcMsg.");
		free(ctx);
		return NULL;
	}

	ctx->unparser = IrcMsgUnparser_New(log);
	if (ctx->unparser == NULL)
	{
		LOG_ERROR(log, "Failed to create IrcMsgUnparser.");
		free(ctx->msg);
		free(ctx);
		return NULL;
	}

	ctx->writer = IrcMsgWriter_New(log, socket);
	if (ctx->writer == NULL)
	{
		LOG_ERROR(log, "Failed to create IrcMsgWriter.");
		free(ctx->unparser);
		free(ctx->msg);
		free(ctx);
		return NULL;
	}

	return ctx;
}

static void SendMsgContext_Delete(void* context)
{
	SendMsgContext* ctx = (SendMsgContext*) context;

	IrcMsgWriter_Delete(ctx->writer);
	free(ctx);
}

static TaskStatus SendMessages(void* context)
{
	SendMsgContext* ctx = (SendMsgContext*) context;

	const char* rawMsg = IrcMsgUnparser_Unparse(ctx->unparser, ctx->msg);

	free(ctx->msg);
	ctx->msg = NULL;

	if (rawMsg == NULL)
	{
		LOG_ERROR(ctx->log, "Failure to unparse message");
		return TaskStatus_Failed;
	}

	LOG_DEBUG(ctx->log, "Sending message: %s", rawMsg);

	if (!IrcMsgWriter_Write(ctx->writer, rawMsg))
	{
		LOG_ERROR(ctx->log, "Failure to write message");
		return TaskStatus_Failed;
	}

	return TaskStatus_Done;
}
