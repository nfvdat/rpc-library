#ifndef LOCATE_MESSAGES_H
#define LOCATE_MESSAGES_H

#include <string>

#include "message.h"
#include "protocol_macros.h"
#include "arg.h"

MAKE_STATUS_MESSAGE(LocateFailure, MessageType::LOCATE_FAILURE);

/*
struct LocateRequest {
    std::string function_name;
    int *arg_types;  // following assignment protocol
};
*/

class LocateRequest {
private:
    std::string function_name; 
    int *arg_types;  // according to standard format

public:
    LocateRequest(const std::string& function_name, int* arg_types);
    ~LocateRequest();

    // Disallowing for now, implement these if needed.
    LocateRequest(const LocateRequest& other) = delete;
    LocateRequest& operator=(const LocateRequest& other) = delete;

    std::string getFunctionName() const;
    // Don't use pointers returned by these functions after this object has
    // been destroyed, since it will delete its copy of the arguments
    // then.
    int* getArgTypes() const;
};

template<> class Serializer<LocateRequest> {
public:
    static RawMessage serialize(const LocateRequest& message) {
        std::string buffer;
        buffer += static_cast<char>(MessageType::LOCATE_REQUEST);
        buffer += encode_integer(message.getFunctionName().size());
        buffer += message.getFunctionName();

        int *arg_types = message.getArgTypes();
        int num_args = numberOfArgs(arg_types);
        buffer += encode_integer(num_args);

        for(int i = 0; i < num_args; i++) {
            buffer += encode_integer(arg_types[i]);
        }

        return RawMessage(buffer.c_str(), buffer.size());
    }

    static LocateRequest* deserialize(const RawMessage& raw_message) {
        //assert(raw_message.getMessageType() == MessageType::LOCATE_REQUEST);

        std::string buffer = raw_message.getString();
        int pos = 1;  // skip message type
        std::string function_name = decode_string(buffer, &pos);
        int num_args = decode_integer(buffer, &pos);

        int* arg_types = new int[num_args + 1];
        arg_types[num_args] = 0;
        for (int i = 0; i < num_args; i++) {
            arg_types[i] = decode_integer(buffer, &pos);
        }

        //assert(pos == (int) buffer.size());

        LocateRequest* message = new LocateRequest(function_name, arg_types);
        delete [] arg_types;

        return message;
    }
};

struct LocateSuccess {
    std::string server_hostname;
    int server_port;
};

template<> class Serializer<LocateSuccess> {
public:
    static RawMessage serialize(const LocateSuccess& message) {
        std::string buffer;
        buffer += static_cast<char>(MessageType::LOCATE_SUCCESS);
        buffer += encode_integer(message.server_hostname.size());
        buffer += message.server_hostname;
        buffer += encode_integer(message.server_port);

        return RawMessage(buffer.c_str(), buffer.size());
    }

    static LocateSuccess* deserialize(const RawMessage& raw_message) {
        //assert(raw_message.getMessageType() == MessageType::LOCATE_SUCCESS);

        LocateSuccess message;
        std::string buffer = raw_message.getString();
        int pos = 1;  // skip message type
        message.server_hostname = decode_string(buffer, &pos);
        message.server_port     = decode_integer(buffer, &pos);

        //assert(pos == (int) buffer.size());
        return new LocateSuccess{ message };
    }
};

#endif
