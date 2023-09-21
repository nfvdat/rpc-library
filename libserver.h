#ifndef LIBSERVER_H
#define LIBSERVER_H

#include <map>

#include "signature.h"
#include "rpc.h"
#include "tcp_server.h"
#include "tcp_client.h"

typedef std::map<Signature, skeleton> ServerDatabase;

// Serves the given skeleton on the given TCP socket. Returns 0 on normal
// termination by binder.
int serve(const ServerDatabase& database,
        TcpServer& server,
        TcpClient* binder);

#endif
