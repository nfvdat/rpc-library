#include "rpc.h"

#include <string>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <sstream>
#include <cstring>

#include "tcp_server.h"
#include "tcp_client.h"
#include "register_messages.h"
#include "execute_messages.h"
#include "libserver.h"
#include "locate_messages.h"
#include "terminate_messages.h"

using namespace std;

struct host_and_port {
    char *host, *port;
};

// Set this with the status to be returned if errors occur, but in the caller
// code, only check its value if we know that an error occurred (otherwise
// it could contain old values).
// See [StatusCodes] above.
int err_code;

// Connection to the binder to register RPCs.
TcpClient* binder;

// Socket for the current server to serve clients.
TcpServer* local_server;
// Database from function signatures to skeletons in this server.
ServerDatabase database;

// Returns a NULL pointer if failed to connect, check [err_code] for code.
TcpClient* connect_to_binder() {
    char *binder_host = getenv("BINDER_ADDRESS");
    if (binder_host == NULL) {
        //cerr << "BINDER_ADDRESS not set" << endl;
        return NULL;
    }

    char *binder_port = getenv("BINDER_PORT");
    if (binder_port == NULL) {
        //cerr << "BINDER_PORT not set" << endl;
        return NULL;
    }

    host_and_port binder_host_and_port = {
        .host = binder_host,
        .port = binder_port
    };

    char* host = binder_host_and_port.host;

    stringstream ss(binder_host_and_port.port);
    int port;
    ss >> port;

    try {
        return new TcpClient(string(host), port);
    } catch (const Error& e) {
        //cerr << "Connection to binder failed: " << e.getReason() << endl;

        // We don't need to set an error code here since this is initialization
        // code. We should set one on the exposed functions, and then use
        // the code to indicate that the connection never succeeded
        // (since the binder is NULL).
        return NULL;
    }
}

// Returns NULL on failure.
TcpServer* create_server_socket() {
    try {
        return new TcpServer();
    } catch (const Error& e) {
        //cerr << "Failure to create server socket: " << e.getReason() << endl;
        return NULL;
    }
}

int rpcInit() {
    binder = connect_to_binder();
    if (!binder)
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);
    local_server = create_server_socket();
    if (!local_server)
        return static_cast<int>(StatusCodes::SERVER_SOCKET_CREATION_ERROR);

    return 0;
}

int rpcRegister(const char *name, int *argTypes, skeleton f) {
    if (!local_server)
        return static_cast<int>(StatusCodes::SERVER_SOCKET_CREATION_ERROR);
    if (!binder)
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);

    pair<string, int> local_server_address = local_server->hostAndPort();

    Register message(local_server_address.first, local_server_address.second,
            string(name), argTypes);
    RawMessage binder_message =
        Serializer<Register>::serialize(message);
    try {
        //cerr << "Will send: " << binder_message.getString() << endl;
        binder->send(
                binder_message.getString().c_str(),
                binder_message.getString().size());

        string response = binder->recv();
        RawMessage raw_response(response.c_str(), response.size());
        switch (raw_response.getMessageType()) {
            case MessageType::REGISTER_SUCCESS:
                {
                    auto signature = signature_of_arg_types(
                            string(name), argTypes);
                    database[signature] = f;
                    return 0;
                }
            case MessageType::REGISTER_FAILURE:
                {
                    unique_ptr<RegisterFailure> message{
                        Serializer<RegisterFailure>::deserialize(
                                raw_response)};
                    return message->status;
                }
            default:
                {
                    return static_cast<int>(
                            StatusCodes::UNEXPECTED_MESSAGE);
                }
        }
    } catch (const Error& e) {
        /*cerr << "Failed to register with binder: "
            << e.getReason() << endl;*/
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);
    }
}

int rpcExecute() {
    if(!binder)
        binder = connect_to_binder();
    if(!binder)
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);
    return serve(database, *local_server, binder);
}

void copy_output_arguments_back(const Execute& result, void** args_dest) {
    int* arg_types = result.getArgTypes();
    void** args_src = result.getArgs();
    int num_args   = numberOfArgs(arg_types);
    for (int i = 0; i < num_args; i++) {
        if (!isOnlyOutput(arg_types[i]) && !isInputOutput(arg_types[i]))
            continue;

        size_t size_of_arg_type = getSizeOfArgType(getArgType(arg_types[i]));
        int arg_length = getArgLength(arg_types[i]);
        size_t nbytes = size_of_arg_type * arg_length;

        memcpy(args_dest[i], args_src[i], nbytes);
    }
}

int rpcCall(const char* name, int* argTypes, void** args) {
    // Quick and dirty lazy initialization.
    if (!binder)
        binder = connect_to_binder();
    if (!binder)
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);

    string server_host;
    int server_port;
    try {
        LocateRequest locate(string(name), argTypes);
        string locate_buffer =
            Serializer<LocateRequest>::serialize(locate)
            .getString();
        binder->send(locate_buffer.c_str(), locate_buffer.size());

        string response = binder->recv();
        RawMessage raw_response(response.c_str(), response.size());
        switch (raw_response.getMessageType()) {
            case MessageType::LOCATE_FAILURE:
                {
                    return static_cast<int>(StatusCodes::LOCATE_FAILURE);
                }
            case MessageType::LOCATE_SUCCESS:
                {
                    unique_ptr<LocateSuccess> locate_success{
                        Serializer<LocateSuccess>::deserialize(raw_response)};
                    server_host = locate_success->server_hostname;
                    server_port = locate_success->server_port;
                    break;
                }
            default:
                {
                    return static_cast<int>(StatusCodes::UNEXPECTED_MESSAGE);
                }
        }
    } catch (const Error& e) {
        //cerr << "Error querying binder (locate): " << e.getReason() << endl;
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);
    }

    RawMessage response;
    try {
        TcpClient connection(server_host, server_port);

        Execute message(string(name), argTypes, args);
        string message_buf =
            Serializer<Execute>::serialize(message).getString();
        connection.send(message_buf.c_str(), message_buf.size());

        string response_buf = connection.recv();
        response = RawMessage(response_buf.c_str(), response_buf.size());
    } catch (const Error& e) {
        //cerr << "Failed to communicate to server: " << e.getReason() << endl;
        return static_cast<int>(StatusCodes::SERVER_CONNECTION_ERROR);
    }

    switch (response.getMessageType()) {
        case MessageType::EXECUTE: // meaning success
            {
                unique_ptr<Execute> execute_result{
                    Serializer<Execute>::deserialize(response)};
                copy_output_arguments_back(*execute_result, args);
                return 0;
            }
        case MessageType::EXECUTE_FAILURE:
            {
                unique_ptr<ExecuteFailure> failure{
                    Serializer<ExecuteFailure>::deserialize(response)};
                return failure->status;
            }
        default:
            {
                return static_cast<int>(StatusCodes::UNEXPECTED_MESSAGE);
            }
    }
}

int rpcTerminate() {
    if (!binder)
        binder = connect_to_binder();
    if (!binder)
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);

    Terminate message{0};  // useless status
    string buffer = Serializer<Terminate>::serialize(message).getString();
    try {
        binder->send(buffer.c_str(), buffer.size());
        string response_s = binder->recv();
        RawMessage response(response_s.c_str(), response_s.size());
        switch (response.getMessageType()) {
            case MessageType::TERMINATE_SUCCESS:
                {
                    return 0;
                }
            case MessageType::TERMINATE_FAILURE:
                {
                    unique_ptr<TerminateFailure> failure{
                        Serializer<TerminateFailure>::deserialize(response)};
                    return failure->status;
                }
            default:
                {
                    return static_cast<int>(StatusCodes::UNEXPECTED_MESSAGE);
                }
        }
    } catch (const Error& e) {
        /*cerr << "Failure sending termination to binder: " << e.getReason()
            << endl;*/
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);
    }
}
