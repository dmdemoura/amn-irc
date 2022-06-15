#include "send_msg_task.h"

#include "irc_msg_writer.h"

#include <stdlib.h>

typedef struct SendMsgContext
{
	const Logger* log;
	const char* msg;
	IrcMsgWriter* writer;
}
SendMsgContext;

static SendMsgContext* SendMsgContext_New(const Logger* log, int socket, const char* msg);
static void SendMsgContext_Delete(void* context);
static void SendMessages(void* context);

Task* SendMsgTask_New(const Logger* log, int socket, const char* msg)
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

static SendMsgContext* SendMsgContext_New(const Logger* log, int socket, const char* msg)
{
	SendMsgContext* ctx = malloc(sizeof(SendMsgContext));
	if (ctx == NULL)
	{
		LOG_ERROR(log, "Failed to allocate SendMsgContext.");
		return NULL;
	}

	ctx->log = log;
	ctx->msg = msg;

	ctx->writer = IrcMsgWriter_New(log, socket);
	if (ctx->writer == NULL)
	{
		LOG_ERROR(log, "Failed to create IrcMsgWriter.");
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

static void SendMessages(void* context)
{
	SendMsgContext* ctx = (SendMsgContext*) context;

	IrcMsgWriter_Write(ctx->writer, ctx->msg);
}
