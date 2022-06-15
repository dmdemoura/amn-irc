#include "accept_conn_task.h"

#include "receive_msg_task.h"

#include <stdlib.h>

#include <sys/socket.h>
#include <unistd.h>

typedef struct AcceptConnContext
{
	const Logger* log;
	TaskQueue* tasks;
	int socket;
}
AcceptConnContext;

static void WaitForConnections(void* context);

Task* AcceptConnTask_New(const Logger* log, TaskQueue* tasks, int socket)
{
	AcceptConnContext* context = malloc(sizeof(AcceptConnContext));
	if (context == NULL)
	{
		return NULL;
	}

	context->log = log;
	context->tasks = tasks;
	context->socket = socket;

	Task* self = Task_Create(WaitForConnections, context, free);
	if (self == NULL)
	{
		free(context);
		return NULL;
	}

	return self;
}

static void WaitForConnections(void* arg)
{
	AcceptConnContext* ctx = (AcceptConnContext*) arg;

	while(1)
	{
		LOG_DEBUG(ctx->log, "Waiting for connection");

		int clientSocket = accept(ctx->socket, NULL, NULL);
		if (clientSocket == -1)
		{
			LOG_ERROR(ctx->log, "Failed to accept connection.");
			break;
		}

		Task* receiveTask = ReceiveMsgTask_New(ctx->log, ctx->tasks, clientSocket);	
		if (receiveTask == NULL)
		{
			LOG_ERROR(ctx->log, "Failed to create ReceiveMsgTask.");

			if (close(ctx->socket) != 0)
			{
				LOG_ERROR(ctx->log, "Failed to close client socket.");
			}

			break;
		}

		if (!TaskQueue_Push(ctx->tasks, receiveTask))
		{
			LOG_ERROR(ctx->log, "Failed to add task to queue.");
			break;
		}
	}

	if (close(ctx->socket) != 0)
	{
		LOG_ERROR(ctx->log, "Failed to close listen socket.");
	}
}
