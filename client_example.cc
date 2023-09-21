#include "rpc.h"

#include <iostream>
#include <cassert>

using namespace std;

bool cmp(float a, float b, float eps = 1e-9) {
    if (a + eps >= b) {
        if (b + eps >= a)
            return 0;
        return 1;
    }
    return -1;
}

int main() {
    int ret;

    int arg_types[3];
    arg_types[0] = (1 << ARG_OUTPUT) | (ARG_CHAR << 16);
    arg_types[1] = (1 << ARG_INPUT) | (1 << ARG_OUTPUT)
        | (ARG_DOUBLE << 16) | 42;
    arg_types[2] = 0;

    char arg1 = 'a';
    double args2[42] = { 0 };
    args2[0] = 1.5;
    args2[41] = 5.1;
    args2[3] = -123.321;

    void* args[2] = { &arg1, args2 };

    ret = rpcCall("my_function123", arg_types, args);
    cerr << "rpcCall result: " << ret << endl;
    assert(ret == 0);

    cerr << "arg1: " << arg1 << endl;
    cerr << "args2 (nonzero):" << endl;
    for (int i = 0; i < 42; i++) {
        if (args2[i] > 1e-9 || args2[i] < -1e-9)
            cerr << "(" << i << ") " << args2[i] << endl;
    }

    assert(arg1 == 'b');
    assert(cmp(args2[0], 1.5) == 0);
    assert(cmp(args2[3], -123.321) == 0);
    assert(cmp(args2[7], -9.1234) == 0);
    assert(cmp(args2[41], 42.45) == 0);

    ret = rpcTerminate();
    assert(ret == 0);
}
