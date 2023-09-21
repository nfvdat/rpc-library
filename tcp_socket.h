#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "error.h"
#include <vector>

/* Class for a TCP socket. Needs a socket number to be constructed
 * (e.g. from [TcpClient] or [TcpServer]), and allows sending and receiving.
 * Closes the connection on the end of the object's lifetime.
 *
 * Assumes every message is preceded by its length on the wire.
 *
 * Uses exceptions.
 */
class TcpSocket {
private:
    const int socket;

public:
    MAKE_ERROR_CLASS(ConnectionClosed);
    MAKE_ERROR_CLASS(SendError);
    MAKE_ERROR_CLASS(RecvError);

    TcpSocket(int socket);
    ~TcpSocket();

    void send(const char* buffer, int length) const;
    std::string recv() const;

    int fd() const;
};

#endif
