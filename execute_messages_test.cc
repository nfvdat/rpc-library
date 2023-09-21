#include "execute_messages.h"
#include "arg.h"

#include <string>
#include <cassert>
#include <memory>
#include <iostream>

using namespace std;

int main() {
    const int length = 42;

    int arg_types[3];
    arg_types[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    arg_types[1] = (1 << ARG_INPUT)  | (ARG_INT << 16) | length;
    arg_types[2] = 0;

    int number = 17;
    int array[length];
    for (int i = 0; i < length; i++)
        array[i] = 2 * length + 1;

    void* args[2];
    args[0] = (void *) &number;
    args[1] = (void *) array;

    size_t sizes[2] = { sizeof(int), length * sizeof(int) };

    string function_name = "doStuff";

    Execute message(function_name, arg_types, args);
    RawMessage serialized = Serializer<Execute>::serialize(message);
    assert(serialized.getMessageType() == MessageType::EXECUTE);

    unique_ptr<Execute> deserialized(
            Serializer<Execute>::deserialize(serialized));
    assert(deserialized->getFunctionName() == function_name);

    int* arg_types_back = deserialized->getArgTypes();
    void** args_back    = deserialized->getArgs();

    int num_args_back = numberOfArgs(arg_types_back);
    assert(num_args_back == 2);
    assert(args_back[num_args_back] == 0);
    for (int i = 0; i < num_args_back; i++) {
        assert(arg_types_back[i] == arg_types[i]);

        int arg_type = arg_types_back[i];
        size_t size_of_arg_type = getSizeOfArgType(getArgType(arg_type));
        int arg_length = getArgLength(arg_type);
        size_t nbytes = size_of_arg_type * arg_length;

        assert(nbytes == sizes[i]);

        string arg_back(static_cast<char*>(args_back[i]), nbytes);
        string orig(static_cast<char*>(args[i]), nbytes);
        assert(arg_back == orig);
    }
}
