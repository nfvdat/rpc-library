#include "register_messages.h"

#include <cassert>
#include <memory>

using namespace std;

int main() {
    const RegisterSuccess succ1 = { 0 };
    const RegisterSuccess succ2 = { 42 };
    const RegisterSuccess succ3 = { -7 };

    for (const auto& msg : { succ1, succ2, succ3 }) {
        RawMessage serialized = Serializer<RegisterSuccess>::serialize(msg);
        assert(serialized.getMessageType() == MessageType::REGISTER_SUCCESS);

        // Test copying.
        RawMessage s1 = serialized;
        serialized = s1;

        unique_ptr<RegisterSuccess> deserialized(
                Serializer<RegisterSuccess>::deserialize(serialized));
        assert(deserialized->status == msg.status);
    }

    const RegisterFailure fail1 = { 0 };
    const RegisterFailure fail2 = { 42 };
    const RegisterFailure fail3 = { -7 };

    for (const auto& msg : { fail1, fail2, fail3 }) {
        RawMessage serialized = Serializer<RegisterFailure>::serialize(msg);
        assert(serialized.getMessageType() == MessageType::REGISTER_FAILURE);

        unique_ptr<RegisterFailure> deserialized(
                Serializer<RegisterFailure>::deserialize(serialized));
        assert(deserialized->status == msg.status);
    }

    const int length = 3;
    int arg_types[length] = { 123, 1165443287, 0 };
    const Register register_msg(
        string("hostname"), 1234567,
        string("my_function_name_123"),
        arg_types);

    {
        RawMessage serialized = Serializer<Register>::serialize(register_msg);
        assert(serialized.getMessageType() == MessageType::REGISTER);

        unique_ptr<Register> deserialized(
                Serializer<Register>::deserialize(serialized));
        assert(deserialized->getServerHostname() ==
                register_msg.getServerHostname());
        assert(deserialized->getServerPort() == register_msg.getServerPort());
        assert(deserialized->getFunctionName() ==
                register_msg.getFunctionName());

        int num_args_back = numberOfArgs(deserialized->getArgTypes());
        assert(num_args_back == length - 1);

        for (int i = 0; i < length; i++)
            assert(deserialized->getArgTypes()[i] ==
                    register_msg.getArgTypes()[i]);
    }
}
