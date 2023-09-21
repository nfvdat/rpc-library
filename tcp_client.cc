#include "tcp_client.h"

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

TcpSocket* connect_create_socket(const string& hostname, int port) {
    addrinfo hints;
    addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype  = SOCK_STREAM;

    stringstream ss;
    ss << port;
    string port_s = ss.str();

    if (int status = getaddrinfo(hostname.c_str(), port_s.c_str(),
                &hints, &res)) {
        //cerr << "Failed to get addr info: " << gai_strerror(status) << endl;
        throw TcpClient::ConnectionError(gai_strerror(status));
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        freeaddrinfo(res);
        //cerr << "Error creating socket: " << strerror(errno) << endl;
        throw TcpClient::ConnectionError(strerror(errno));
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
        freeaddrinfo(res);
        //cerr << "Error connecting to server: " << strerror(errno) << endl;
        throw TcpClient::ConnectionError(strerror(errno));
    }

    freeaddrinfo(res);

    return new TcpSocket(sock);
}

TcpClient::TcpClient(const string& hostname, int port)
    : socket(connect_create_socket(hostname, port)) {}

void TcpClient::send(const char* buffer, int length) const {
    socket->send(buffer, length);
}

string TcpClient::recv() const {
    return socket->recv();
}
