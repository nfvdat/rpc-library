#ifndef EXECUTE_MESSAGES_H
#define EXECUTE_MESSAGES_H

#include <string>
#include <cstring>
#include <iostream>

#include "message.h"
#include "protocol_macros.h"
#include "arg.h"

MAKE_STATUS_MESSAGE(ExecuteFailure, MessageType::EXECUTE_FAILURE);

/* [Execute] can be used both for requesting execution or to respond
 * in the case of success (that is, EXECUTE and EXECUTE_SUCCESS). We can
 * determine what the message means depending on the context.
 *
 * It copies the arguments upon construction, so that it owns the associated
 * memory and can be treated the same way on both ends.
 */
class Execute {
private:
    std::string function_name;
    int* arg_types;  // according to standard format
    void** args;  // constructed on initialization by copy

public:
    /* [arg_ptrs] is an array of pointers to the arguments, each of which
     * we will copy.
     */
    Execute(const std::string& function_name, int* arg_types, void** arg_ptrs);
    ~Execute();

    // Disallowing for now, implement these if needed.
    Execute(const Execute& other) = delete;
    Execute& operator=(const Execute& other) = delete;

    std::string getFunctionName() const;
    // Don't use pointers returned by these functions after this object has
    // been destroyed, since it will delete its copy of the arguments
    // then.
    int* getArgTypes() const;
    void** getArgs() const;
};

template<> class Serializer<Execute> {
public:
    static RawMessage serialize(const Execute& message) {
        std::string buffer;
        buffer += static_cast<char>(MessageType::EXECUTE);
        buffer += encode_integer(message.getFunctionName().size());
        buffer += message.getFunctionName();

        int* arg_types = message.getArgTypes();
        void** args = message.getArgs();

        int num_args = numberOfArgs(arg_types);
        buffer += encode_integer(num_args);

        for (int i = 0; i < num_args; i++) {
            buffer += encode_integer(arg_types[i]);
        }

        for (int i = 0; i < num_args; i++) {
            int arg_type = arg_types[i];
            size_t size_of_arg_type = getSizeOfArgType(getArgType(arg_type));
            int arg_length = getArgLength(arg_type);
            size_t nbytes = size_of_arg_type * arg_length;

            std::string arg_data(static_cast<char*>(args[i]), nbytes);
            buffer += arg_data;
        }

        return RawMessage(buffer.c_str(), buffer.size());
    }

    static Execute* deserialize(const RawMessage& raw_message) {
        //assert(raw_message.getMessageType() == MessageType::EXECUTE);

        std::string buffer = raw_message.getString();
        int pos = 1;  // skip message type
        std::string function_name = decode_string(buffer, &pos);
        int num_args = decode_integer(buffer, &pos);

        int* arg_types = new int[num_args + 1];
        arg_types[num_args] = 0;
        for (int i = 0; i < num_args; i++) {
            arg_types[i] = decode_integer(buffer, &pos);
        }

        void** args = new void*[num_args];
        for (int i = 0; i < num_args; i++) {
            int arg_type = arg_types[i];
            size_t size_of_arg_type = getSizeOfArgType(getArgType(arg_type));
            int arg_length = getArgLength(arg_type);
            size_t nbytes = size_of_arg_type * arg_length;

            args[i] = (void *) malloc(nbytes);
            memcpy(args[i], buffer.c_str() + pos, nbytes);
            pos += nbytes;
        }

        //assert(pos == (int) buffer.size());

        Execute* message = new Execute(function_name, arg_types, args);
        delete [] arg_types;
        for (int i = 0; i < num_args; i++)
            free(args[i]);
        delete [] args;
        return message;
    }
};

#endif
