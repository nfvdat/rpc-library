#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "tcp_socket.h"

#include <string>
#include <memory>

/* Class to handle a TCP client socket.
 * It assumes that every message is starts with an integer indicating the
 * length of the rest of the message.
 *
 * Uses exceptions for failures, so catch them if you need to.
 */
class TcpClient {
private:
    std::unique_ptr<TcpSocket> socket;

public:
    MAKE_ERROR_CLASS(ConnectionError);

    TcpClient(const std::string& hostname, int port);
    void send(const char* buffer, int length) const;
    std::string recv() const;
};

#endif
