#include "tcp_socket.h"

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
#include <sstream>

using namespace std;

TcpSocket::TcpSocket(int socket) : socket(socket) {}

TcpSocket::~TcpSocket() {
    close(socket);
}

void TcpSocket::send(const char* buffer, int length) const {
    auto handle_error = []() {
        if (errno == EPIPE)
            throw TcpSocket::ConnectionClosed("Connection closed");
        else
            throw TcpSocket::SendError(strerror(errno));
    };

    if (::send(socket, &length, sizeof(int), MSG_NOSIGNAL) == -1)
        handle_error();

    int bytes_sent = 0;
    while (bytes_sent < length) {
        int now_sent = ::send(socket, buffer + bytes_sent,
                length - bytes_sent, MSG_NOSIGNAL);
        if (now_sent == -1)
            handle_error();
        bytes_sent += now_sent;
    }
}

string TcpSocket::recv() const {
    auto handle_error = [](int return_code) {
        if (return_code == 0)
            throw TcpSocket::ConnectionClosed("Connection closed");
        else
            throw TcpSocket::RecvError(strerror(errno));
    };

    int length;
    int ret = ::recv(socket, &length, sizeof(int), 0);
    if (ret <= 0)
        handle_error(ret);

    char* response = new char[length + 1];
    int bytes_received = 0;
    while (bytes_received < length) {
        int now_received = ::recv(socket, response + bytes_received,
                length - bytes_received, 0);

        if (now_received <= 0) {
            delete [] response;
            handle_error(now_received);
        }

        bytes_received += now_received;
    }

    response[length] = '\0';
    string response_s(response, length);
    delete [] response;
    return response_s;
}

int TcpSocket::fd() const {
    return socket;
}
