#include "signature.h"
#include "arg.h"

#include <iostream>
#include <cassert>
#include <string>
#include <tuple>

using namespace std;

int main() {
    const int length = 42;
    int arg_types[3];
    arg_types[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_OUTPUT)
        | (ARG_FLOAT << 16) | length;
    arg_types[2] = 0;

    string name = "my_fn_12";

    Signature signature = signature_of_arg_types(name, arg_types);
    Signature expected{
        name,
        {Argument{ScalarOrArray::SCALAR, InOutBoth::OUTPUT, DataType::INT},
         Argument{ScalarOrArray::ARRAY , InOutBoth::BOTH  , DataType::FLOAT}}};
    assert(signature == expected);
}
