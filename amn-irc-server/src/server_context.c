#include "server_context.h"

#include <stddef.h>
#include <stdlib.h>

struct ServerContext
{
	const Logger* log;
	int* clientSockets;
	size_t numClients;
};

ServerContext* ServerContext_New(const Logger* log)
{
	ServerContext* self = malloc(sizeof(ServerContext));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocated ServerContext"); 
		return NULL;
	}

	self->log = log;
	self->clientSockets = NULL;
	self->numClients = 0;

	return self;
}

void ServerContext_Delete(ServerContext* self)
{
	free(self->clientSockets);
	free(self);
}

bool ServerContext_AddClient(ServerContext* self, int clientSocket)
{
	self->numClients += 1;

	int* clients = realloc(self->clientSockets, sizeof(int) * self->numClients);
	if (clients == NULL)
	{
		LOG_ERROR(self->log, "Failure to (re)allocate clientSockets.");
		return false;
	}

	clients[self->numClients - 1] = clientSocket;
	
	return true;
}

ClientList ServerContext_GetAllClients(ServerContext* self)
{
	ClientList list =  {
		.clientSockets = self->clientSockets,
		.numClients = self->numClients
	};

	return list;
}
