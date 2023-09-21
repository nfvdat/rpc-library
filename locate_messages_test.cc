#include "locate_messages.h"

#include <cassert>
#include <memory>
#include <iostream>

using namespace std;

int main() {
    const LocateSuccess success = { "hostname", 123456789  };
    
    {
        RawMessage serialized =
            Serializer<LocateSuccess>::serialize(success);
        assert(serialized.getMessageType() == MessageType::LOCATE_SUCCESS);

        unique_ptr<LocateSuccess> deserialized(
                Serializer<LocateSuccess>::deserialize(serialized));
        assert(deserialized->server_hostname == success.server_hostname);
        assert(deserialized->server_port     == success.server_port);
    }

    const LocateFailure fail1 = { 0 };
    const LocateFailure fail2 = { 42 };
    const LocateFailure fail3 = { -7 };

    for (const auto& msg : { fail1, fail2, fail3 }) {
        RawMessage serialized = Serializer<LocateFailure>::serialize(msg);
        assert(serialized.getMessageType() == MessageType::LOCATE_FAILURE);

        unique_ptr<LocateFailure> deserialized(
                Serializer<LocateFailure>::deserialize(serialized));
        assert(deserialized->status == msg.status);
    }

    const int length = 42;

    int arg_types[3];
    arg_types[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    arg_types[1] = (1 << ARG_INPUT)  | (ARG_INT << 16) | length;
    arg_types[2] = 0;

    string function_name = "doStuff";

    LocateRequest message(function_name, arg_types);
    RawMessage serialized = Serializer<LocateRequest>::serialize(message);
    assert(serialized.getMessageType() == MessageType::LOCATE_REQUEST);

    unique_ptr<LocateRequest> deserialized(
        Serializer<LocateRequest>::deserialize(serialized));
    assert(deserialized->getFunctionName() == function_name);

    int* arg_types_back = deserialized->getArgTypes();

    assert(arg_types_back != NULL);

    int num_args_back = numberOfArgs(arg_types_back);
    assert(num_args_back == 2);
    for (int i = 0; i < num_args_back; i++) {
        assert(arg_types_back[i] == arg_types[i]);
    }
}
