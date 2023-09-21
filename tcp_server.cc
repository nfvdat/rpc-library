#include "tcp_server.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <pthread.h>
#include <queue>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

using namespace std;

TcpServer::TcpServer() {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    addrinfo *servinfo;
    if (int err = getaddrinfo(NULL, "0", &hints, &servinfo))
        throw ServerCreation(strerror(err));

    addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((listener = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
            /*cerr << "Warning: attempt to create listener socket failed: "
                << strerror(errno) << endl;
            cerr << "Trying again" << endl;*/
            continue;
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
            /*cerr << "Warning: attempt to bind listener failed: "
                << strerror(errno) << endl;
            cerr << "Trying again" << endl;*/
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
        throw ServerCreation("Failed to bind");

    if (listen(listener, 128) == -1)
        throw ServerCreation(strerror(errno));

    char hostname[255];
    if (gethostname(hostname, 255)) {
        close(listener);
        throw ServerCreation(strerror(errno));
    }

    int port = -1;
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(listener, (sockaddr *) &sin, &len) == -1) {
        close(listener);
        throw ServerCreation(strerror(errno));
    } else {
        port = ntohs(sin.sin_port);
    }

    this->hostname = hostname;
    this->port = port;

    FD_ZERO(&fd_master);
    FD_SET(listener, &fd_master);
    fd_max = listener;
}

TcpServer::~TcpServer() {
    // The client connections will close themselves.
    close(listener);
}

pair<string, int> TcpServer::hostAndPort() const {
    return make_pair(hostname, port);
}

void TcpServer::acceptIfAny() {
    fd_set read_fds = fd_master;
    if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1)
        throw SelectError(strerror(errno));

    if (FD_ISSET(listener, &read_fds)) {
        int newfd = accept(listener, NULL, NULL);
        if (newfd == -1) {
            //cerr << "Error accepting connection: " << strerror(errno) << endl;
        } else {
            FD_SET(newfd, &fd_master);
            fd_max = max(fd_max, newfd);
            //assert(connections.count(newfd) == 0);
            connections.emplace(newfd,
                    std::unique_ptr<TcpSocket>(new TcpSocket(newfd)));
        }
    }
}

vector<const TcpSocket*> TcpServer::connectionsWithData() {
    fd_set read_fds = fd_master;
    if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1)
        throw SelectError(strerror(errno));

    vector<const TcpSocket*> sockets;
    for (int fd = 0; fd <= fd_max; fd++) {
        if (fd == listener) continue;

        if (FD_ISSET(fd, &read_fds)) {
            // [at] throws an exception if not found, which we want here, since
            // it should never happen.
            sockets.push_back(connections.at(fd).get());
        }
    }

    return sockets;
}

void TcpServer::connectionClosed(const TcpSocket* client) {
    int fd = client->fd();
    close(fd);
    connections.erase(fd);
    FD_CLR(fd, &fd_master);
}

TcpSocket* TcpServer::releaseConnection(const TcpSocket* client) {
    int fd = client->fd();
    FD_CLR(fd, &fd_master);
    TcpSocket* socket = connections[fd].release();
    connections.erase(fd);
    return socket;
}
