#include "libserver.h"
#include "message.h"
#include "execute_messages.h"
#include "tcp_client.h"
#include "tcp_server.h"
#include "terminate_messages.h"

#include <iostream>
#include <tuple>
#include <pthread.h>

using namespace std;

volatile int total_threads = 0;
pthread_mutex_t total_threads_lock;
volatile bool terminating = false;

void inc_threads(int val) {
    pthread_mutex_lock(&total_threads_lock);
    total_threads += val;
    pthread_mutex_unlock(&total_threads_lock);
}

RawMessage handle_execute(const Execute& message,
        const ServerDatabase& database) {
    Signature signature = signature_of_arg_types(
            message.getFunctionName(), message.getArgTypes());

    if (!database.count(signature)) {
        ExecuteFailure failure{static_cast<int>(
                StatusCodes::UNEXPECTED_MESSAGE)};
        return Serializer<ExecuteFailure>::serialize(failure);
    }

    skeleton f = database.at(signature);

    int ret = f(message.getArgTypes(), message.getArgs());
    if (ret != 0) {
        ExecuteFailure failure{ret};
        return Serializer<ExecuteFailure>::serialize(failure);
    }

    // Construct an [Execute] message that stands for "execute success".
    // Presumably the skeleton will mutate any output parameters as needed,
    // so we construct the message the same way with the original pointers.
    Execute success(message.getFunctionName(), message.getArgTypes(),
            message.getArgs());
    return Serializer<Execute>::serialize(success);
}

void* serve_client_thread(void* pargs) {
    auto* args =
        (tuple<const ServerDatabase&, TcpSocket*>*)
        pargs;
    const ServerDatabase& database   = get<0>(*args);
    TcpSocket*            client_ptr = get<1>(*args);
    const TcpSocket&      client     = *client_ptr;

    while (true) {
        string buffer;
        try {
            buffer = client.recv();
        } catch (const Error& e) {
            break;
        }

        RawMessage raw_message(buffer.c_str(), buffer.size());

        if (raw_message.getMessageType() != MessageType::EXECUTE) {
            ExecuteFailure failure{
                static_cast<int>(StatusCodes::UNEXPECTED_MESSAGE)};
            string failure_buf =
                Serializer<ExecuteFailure>::serialize(failure)
                .getString();
            try {
                client.send(buffer.c_str(), buffer.size());
            } catch (const Error& e) {
                break;
            }
        }

        unique_ptr<Execute> message{
            Serializer<Execute>::deserialize(raw_message)};
        RawMessage response = handle_execute(*message, database);

        try {
            client.send(response.getString().c_str(),
                    response.getString().size());
        } catch (const Error& e) {
            break;
        }
    }

    delete args;
    delete client_ptr;

    inc_threads(-1);
    pthread_exit(NULL);
}

void* client_listen_thread(void* pargs) {
    auto* args =
          (tuple<TcpServer&, const ServerDatabase&>*)
          pargs;
    TcpServer&            server     = get<0>(*args);
    const ServerDatabase& database   = get<1>(*args);

    while(true) {
        //cerr << "Server loop" << endl;

        try {
            server.acceptIfAny();
        } catch (const Error& e) {
            //cerr << "Error accepting: " << e.getReason() << endl;
        }

        vector<const TcpSocket*> clients = server.connectionsWithData();
        for (const auto* client : clients) {
            // Make server forget about it so we don't interfere with thread.
            TcpSocket* socket = server.releaseConnection(client);
            
            pthread_t thread;
            inc_threads(1);
            pthread_create(&thread, NULL, serve_client_thread,
                new tuple<const ServerDatabase&,
                    TcpSocket*>{database, socket});
        }
    }
}

void* binder_listen_thread(void* pargs) {
    auto* args =
         (tuple<TcpClient*>*)
         pargs;
    TcpClient*            binder   = get<0>(*args);

    while(true) {
        try {
            string response = binder->recv();
            RawMessage raw_response(response.c_str(), response.size());

            if(raw_response.getMessageType() == MessageType::TERMINATE) {
                terminating = true;
                inc_threads(-1);
                pthread_exit(NULL);
            }
        } catch (const Error& e) {
            terminating = true;
            inc_threads(-1);
            pthread_exit(NULL);
        }
    }
}

int serve(const ServerDatabase& database, TcpServer& server, TcpClient* binder) {
    pthread_t listen_to_binder, listen_to_clients;
    inc_threads(2);
    pthread_create(&listen_to_binder, NULL, binder_listen_thread,
            new tuple<TcpClient*>{binder});
    pthread_create(&listen_to_clients, NULL, client_listen_thread,
            new tuple<TcpServer&,
                        const ServerDatabase&>{server, database});

    while(!terminating or total_threads > 1) {}

    // Now, the only thread running would be listen_to_clients.
    pthread_cancel(listen_to_clients);
    pthread_join(listen_to_clients, NULL);

    try {
        TerminateSuccess success{ 0 };
        string success_buffer =
            Serializer<TerminateSuccess>::serialize(success)
                        .getString();
        binder->send(success_buffer.c_str(), success_buffer.size());
    } catch (const Error& e) {
        //cerr << "Error querying binder (locate): " << e.getReason() << endl;
        return static_cast<int>(StatusCodes::BINDER_CONNECTION_ERROR);
    }

    //cerr << "Halting server" << endl;

    return 0;
}
