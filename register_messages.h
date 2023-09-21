#ifndef REGISTER_MESSAGES_H
#define REGISTER_MESSAGES_H

#include <string>
#include <iostream>

#include "message.h"
#include "protocol_macros.h"
#include "arg.h"

MAKE_STATUS_MESSAGE(RegisterSuccess, MessageType::REGISTER_SUCCESS);
MAKE_STATUS_MESSAGE(RegisterFailure, MessageType::REGISTER_FAILURE);

// This class is similar to [Execute].
class Register {
private:
    std::string server_hostname;
    int server_port;
    std::string function_name;
    int* arg_types;  // according to standard format

public:
    Register(const std::string& _server_hostname, int _server_port,
            const std::string& _function_name, int* _arg_types);
    ~Register();

    // Disallowing for now, implement these if needed.
    Register(const Register& other) = delete;
    Register& operator=(const Register& other) = delete;

    std::string getServerHostname() const;
    int getServerPort() const;
    std::string getFunctionName() const;
    // Don't use pointers returned by these functions after this object has
    // been destroyed, since it will delete its copy of the arguments
    // then.
    int* getArgTypes() const;
};

template<> class Serializer<Register> {
public:
    static RawMessage serialize(const Register& message) {
        std::string buffer;
        buffer += static_cast<char>(MessageType::REGISTER);
        buffer += encode_integer(message.getServerHostname().size());
        buffer += message.getServerHostname();
        buffer += encode_integer(message.getServerPort());
        buffer += encode_integer(message.getFunctionName().size());
        buffer += message.getFunctionName();

        int* arg_types = message.getArgTypes();
        int num_args = numberOfArgs(arg_types);
        buffer += encode_integer(num_args);

        for (int i = 0; i < num_args; i++)
            buffer += encode_integer(arg_types[i]);

        return RawMessage(buffer.c_str(), buffer.size());
    }

    static Register* deserialize(const RawMessage& raw_message) {
        //assert(raw_message.getMessageType() == MessageType::REGISTER);

        std::string buffer = raw_message.getString();
        int pos = 1;  // skip message type
        std::string server_hostname = decode_string(buffer, &pos);
        int server_port     = decode_integer(buffer, &pos);
        std::string function_name   = decode_string(buffer, &pos);

        int num_args = decode_integer(buffer, &pos);

        int* arg_types = new int[num_args + 1];
        arg_types[num_args] = 0;
        for (int i = 0; i < num_args; i++)
            arg_types[i] = decode_integer(buffer, &pos);

        //assert(pos == (int) buffer.size());

        Register* message = new Register(server_hostname, server_port,
                function_name, arg_types);
        delete [] arg_types;
        return message;
    }
};

#endif
