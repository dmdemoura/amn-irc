#ifndef AMN_SERVER_CONTEXT_H
#define AMN_SERVER_CONTEXT_H

#include "log.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct ClientList
{
	const int* clientSockets;
	size_t numClients;
}
ClientList;

typedef struct ServerContext ServerContext;

ServerContext* ServerContext_New(const Logger* log);
void ServerContext_Delete(ServerContext* self);

bool ServerContext_AddClient(ServerContext* self, int clientSocket);
ClientList ServerContext_GetAllClients(ServerContext* self);

#endif // AMN_SERVER_CONTEXT_H
