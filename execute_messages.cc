#include "execute_messages.h"

#include "arg.h"

#include <cstring>
#include <iostream>

using namespace std;

Execute::Execute(const string& function_name, int* arg_types, void** arg_ptrs) {
    this->function_name = function_name;

    int num_args = numberOfArgs(arg_types);
    this->arg_types = new int[num_args + 1];
    this->arg_types[num_args] = 0;

    args = new void*[num_args];
    for (int i = 0; i < num_args; i++) {
        this->arg_types[i] = arg_types[i];

        size_t size_of_arg_type = getSizeOfArgType(getArgType(arg_types[i]));
        //assert(size_of_arg_type != (size_t) -1);
        int arg_length = getArgLength(arg_types[i]);

        args[i] = (void *) malloc(arg_length * size_of_arg_type);
        memcpy(args[i], arg_ptrs[i], arg_length * size_of_arg_type);
    }
}

Execute::~Execute() {
    int num_args = numberOfArgs(arg_types);
    for (int i = 0; i < num_args; i++) {
        free(args[i]);
    }

    delete [] args;
    delete [] arg_types;
}

string Execute::getFunctionName() const {
    return function_name;
}

int* Execute::getArgTypes() const {
    return arg_types;
}

void** Execute::getArgs() const {
    return args;
}
