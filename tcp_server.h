#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <vector>
#include <map>
#include <utility>
#include <memory>

#include "tcp_socket.h"

/* Class for a TCP Server that can accept new connections and talk to clients.
 * Non-blocking.
 *
 * Uses exceptions for error conditions.
 */
class TcpServer {
private:
    // File descriptor and information for the listener socket.
    int listener;
    std::string hostname;
    int port;

    // Map from file descriptor IDs to client connections.
    std::map<int, std::unique_ptr<TcpSocket>> connections;

    // Fd mask to use in [selet]; keeps track of all open sockets including
    // listener.
    fd_set fd_master;
    int fd_max;

public:
    MAKE_ERROR_CLASS(ServerCreation);
    MAKE_ERROR_CLASS(SelectError);

    TcpServer();
    ~TcpServer();

    std::pair<std::string, int> hostAndPort() const;

    // Accepts at most one new connection.
    void acceptIfAny();
    // Returns all connections that have data to receive. For fairness,
    // the caller should probably serve them all.
    std::vector<const TcpSocket*> connectionsWithData();

    // Call this function to stop checking closed socket.
    void connectionClosed(const TcpSocket* client);

    // Stops managing this socket, but leaves it open. Caller must close it
    // as well as clean up memory.
    TcpSocket* releaseConnection(const TcpSocket* client);
};

#endif
