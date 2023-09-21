#include "rpc.h"

#include <iostream>
#include <cassert>

using namespace std;

int bad(int* arg_types, void** args) {
    assert(false);
}

int function_impl(int* arg_types, void** args) {
    cout << "Hey there!" << endl;

    char* arg1 = (char*) args[0];
    double* args2 = (double*) args[1];

    *arg1 = 'b';
    args2[7] = -9.1234;
    args2[41] = 42.45;

    return 0;
}

int main() {
    int ret;
    ret = rpcInit();
    cerr << "rpcInit: " << ret << endl;
    assert(ret >= 0);

    int arg_types[3];
    arg_types[0] = (1 << ARG_OUTPUT) | (ARG_CHAR << 16);
    arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_OUTPUT)
        | (ARG_DOUBLE << 16) | 42;
    arg_types[2] = 0;

    ret = rpcRegister("my_function123", arg_types, bad);
    cerr << "rpcRegister: " << ret << endl;

    ret = rpcRegister("my_function123", arg_types, function_impl);
    cerr << "rpcRegister: " << ret << endl;
    assert(ret >= 0);

    ret = rpcExecute();
    assert(ret >= 0);
}
