#include "locate_messages.h"

#include "arg.h"

#include <cstring>
#include <iostream>

using namespace std;

LocateRequest::LocateRequest(const string& function_name, int* arg_types) {
    this->function_name = function_name;

    int num_args = numberOfArgs(arg_types);
    this->arg_types = new int[num_args + 1];
    this->arg_types[num_args] = 0;
    for (int i = 0; i < num_args; i++)
        this->arg_types[i] = arg_types[i];
}

LocateRequest::~LocateRequest() {
    delete [] arg_types;
}

string LocateRequest::getFunctionName() const {
        return function_name;
}

int* LocateRequest::getArgTypes() const {
        return arg_types;
}

