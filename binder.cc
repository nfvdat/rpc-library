#include <iostream>
#include <map>
#include <queue>
#include <memory>
#include <set>

#include "register_messages.h"
#include "locate_messages.h"
#include "terminate_messages.h"
#include "message.h"
#include "arg.h"
#include "tcp_server.h"
#include "signature.h"

using namespace std;

struct ServerData {
    string server_hostname;
    int server_port;
    const TcpSocket* server_socket;
};

bool operator<(const ServerData s1, const ServerData s2) {
    return make_pair(s1.server_hostname, s1.server_port) < 
        make_pair(s2.server_hostname, s2.server_port);
}

class BinderDatabase {
private:
    unsigned int time_ = 1;
    set<pair<unsigned int, ServerData>> server_queue;
    map<ServerData, unsigned int> server_to_timestamp;
    map<Signature, set<ServerData>> signature_to_servers;
public:
    BinderDatabase() {}

    RawMessage handleRegister(const Register& message, const TcpSocket* server_socket) {
        /*cerr << "Handling registration of "
            << message.getFunctionName()
            << " with server address "
            << message.getServerHostname()
            << " and port "
            << message.getServerPort()
            << endl;*/
        
        int* arg_types = message.getArgTypes();
        string server_hostname = message.getServerHostname();
        int server_port = message.getServerPort();
        string function_name = message.getFunctionName();
        Signature signature = signature_of_arg_types(function_name, arg_types);

        ServerData server = {server_hostname, server_port, server_socket};

        set<ServerData> servers = signature_to_servers[signature];
        if(servers.find(server) != servers.end()) {
            RegisterSuccess warning{ 1 };
            return Serializer<RegisterSuccess>::serialize(warning);
        }
        signature_to_servers[signature].insert(server);

        unsigned int timestamp = server_to_timestamp[server];
        if(timestamp > 0) {
            server_queue.erase(make_pair(timestamp, server));
        }
        server_to_timestamp[server] = time_;
        server_queue.insert(make_pair(time_++, server));

        RegisterSuccess success{ 0 };
        return Serializer<RegisterSuccess>::serialize(success);
    }

    RawMessage handleLocate(const LocateRequest& message) {
        int* arg_types = message.getArgTypes();
        string function_name = message.getFunctionName();
        Signature signature = signature_of_arg_types(function_name, arg_types);

        if (signature_to_servers.count(signature) &&
                signature_to_servers[signature].empty())
            signature_to_servers.erase(signature);

        if(signature_to_servers.find(signature) ==
            signature_to_servers.end()) {
            LocateFailure failure{ 1 };  // status code not used
            return Serializer<LocateFailure>::serialize(failure);
        }

        ServerData success_server;
        bool found = false;

        set<ServerData> servers = signature_to_servers[signature];
        for(auto p: server_queue) {
            ServerData server = p.second;
            if(servers.count(server)) {
                success_server = server;
                server_queue.erase(p);
                found = true;
                break;
            }
        }

        //assert(found);
        if (!found) {
            LocateFailure failure{ 1 };  // status code not used
            return Serializer<LocateFailure>::serialize(failure);
        }

        /*cerr << "Chose server " << success_server.server_port
            << " with timestamp " << server_to_timestamp[success_server]
            << " and current time " << time_ << endl;*/

        server_to_timestamp[success_server] = time_;
        server_queue.insert(make_pair(time_++, success_server));

        LocateSuccess success = {success_server.server_hostname,
            success_server.server_port};
        return Serializer<LocateSuccess>::serialize(success);
    }

    RawMessage handleTerminate(const Terminate& message) {
        RawMessage error_message;
        bool had_error = false;

        for(auto p: server_to_timestamp) {
            const TcpSocket* server_socket = p.first.server_socket;
            Terminate terminate{ 0 };
            string terminate_buffer = 
                Serializer<Terminate>::serialize(terminate)
                .getString();

            try {
                server_socket->send(terminate_buffer.c_str(),
                        terminate_buffer.size());

                string response = server_socket->recv();
                RawMessage raw_message(response.c_str(), response.size());

                if (raw_message.getMessageType() !=
                        MessageType::TERMINATE_SUCCESS) {
                    had_error = true;
                    TerminateFailure failure{ static_cast<int>(
                            StatusCodes::UNEXPECTED_MESSAGE) };
                    error_message = Serializer<TerminateFailure>::
                        serialize(failure);
                }
            } catch (const Error& e) {
                had_error = true;
                TerminateFailure failure{ static_cast<int>(
                        StatusCodes::SERVER_CONNECTION_ERROR) };
                error_message = Serializer<TerminateFailure>::
                    serialize(failure);
            }
        }

        if (had_error)
            return error_message;

        TerminateSuccess success{ 0 };
        return Serializer<TerminateSuccess>::serialize(
                success);
    }

    void removeIfServer(const TcpSocket* socket) {
        // Tricky deletion code that STL doesn't seem to provide.
        for (auto it = server_queue.begin(); it != server_queue.end(); ) {
            /*cerr << "Checking if delete for port " << it->second.server_port
                << endl;*/
            if (it->second.server_socket == socket) {
                //cerr << "Decided to delete" << endl;
                it = server_queue.erase(it);
            }
            else
                ++it;
        }
        for (auto it = server_to_timestamp.begin();
                it != server_to_timestamp.end(); ) {
            if (it->first.server_socket == socket)
                it = server_to_timestamp.erase(it);
            else
                ++it;
        }
        for (auto& sign_and_servers : signature_to_servers) {
            auto& servers = sign_and_servers.second;
            for (auto it = servers.begin(); it != servers.end(); ) {
                if (it->server_socket == socket)
                    it = servers.erase(it);
                else
                    ++it;
            }
        }
    }
};

int main() {
    TcpServer tcp_server;
    BinderDatabase database;

    pair<string, int> host_and_port = tcp_server.hostAndPort();
    cout << "BINDER_ADDRESS " << host_and_port.first << endl;
    cout << "BINDER_PORT " << host_and_port.second << endl;

    bool running = true;
    while (running) {
        //cerr << "Binder loop" << endl;

        try {
            tcp_server.acceptIfAny();
        } catch (const Error& error) {
            //cerr << "Error accepting: " << error.getReason() << endl;
        }

        vector<const TcpSocket*> clients = tcp_server.connectionsWithData();
        for (const auto* client : clients) {
            string buffer;
            try {
                buffer = client->recv();
            } catch (const TcpSocket::ConnectionClosed& e) {
                //cerr << "Connection closed" << endl;
                database.removeIfServer(client);
                tcp_server.connectionClosed(client);
            } catch (const Error& e) {
                //cerr << "Error receiving: " << e.getReason() << endl;
            }

            /*cerr << "Got message: " << buffer << " (length "
                << buffer.size() << ")" << endl;*/

            if (buffer.empty()) continue;

            //cerr << "Got nonempty message: " << buffer << endl;

            RawMessage raw_message(buffer.c_str(), buffer.size());
            //cerr << "Assembled raw message" << endl;

            RawMessage response;
            switch (raw_message.getMessageType()) {
                case MessageType::REGISTER:
                    {
                        unique_ptr<Register> message{
                            Serializer<Register>::deserialize(raw_message)};
                        response = database.handleRegister(*message, client);
                        break;
                    }
                case MessageType::LOCATE_REQUEST:
                    {
                        unique_ptr<LocateRequest> message{
                            Serializer<LocateRequest>::deserialize(raw_message)};
                        response = database.handleLocate(*message);
                        break;
                    }
                case MessageType::TERMINATE:
                    {
                        unique_ptr<Terminate> message{
                            Serializer<Terminate>::deserialize(raw_message)};
                        response = database.handleTerminate(*message);
                        running = false;
                        break;
                    }
                default:
                    {
                        /*cerr << "Unknown message type "
                            << static_cast<int>(raw_message.getMessageType())
                            << endl;*/
                        continue;
                    }
            }

            try {
                client->send(response.getString().c_str(),
                        response.getString().size());
            } catch (const TcpSocket::ConnectionClosed& e) {
                database.removeIfServer(client);
            } catch (const Error& e) {
                // Ignore.
            }

            if (!running) break;
        }
    }
}
