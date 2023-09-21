#include "arg.h"

// see asst page 2 for argument type integer conventions

int getArgType(int argType) {
    // 2nd byte contains argument type information
    return (argType >> 16) & ((1<<8)-1);
}

int getArgLength(int argType) {
    // Lower 2 bytes specify length of the array
    int length = (argType & ((1<<16)-1));

    // length 0 signifies scalar, so equivalent to 1
    return length == 0 ? 1 : length;
}

int getSizeOfArgType(int type) {
    switch(type) {
        case ARG_CHAR:
            return sizeof(char);
        case ARG_SHORT:
            return sizeof(short);
        case ARG_INT:
            return sizeof(int);
        case ARG_LONG:
            return sizeof(long);
        case ARG_DOUBLE:
            return sizeof(double);
        case ARG_FLOAT:
            return sizeof(float);
        default:
            // todo: throw invalid type exception?
            return -1;
    }
}

int numberOfArgs(int *argTypes) {
    int num = 0;
    // last element in argTypes is 0
    while(*(argTypes + num) != 0) {
        num++;
    }
    return num;
}

int totalSizeOfArgs(int *argTypes) {
    int num = numberOfArgs(argTypes);
    int totalSize = 0;
    for(int i = 0; i < num; i++) {
        int argType = *(argTypes + i);
        int typeSize = getSizeOfArgType(getArgType(argType));
        totalSize += typeSize*getArgLength(argType);
    }
    return totalSize;
}

bool isOnlyInput(int argType) {
    return ((argType >> 30) & 0x3) == 2;
}

bool isOnlyOutput(int argType) {
    return ((argType >> 30) & 0x3) == 1;
}

bool isInputOutput(int argType) {
    return ((argType >> 30) & 0x3) == 3;
}
