#ifndef ARG_H
#define ARG_H

#define ARG_CHAR    1
#define ARG_SHORT   2
#define ARG_INT     3
#define ARG_LONG    4
#define ARG_DOUBLE  5
#define ARG_FLOAT   6

#define ARG_INPUT      31
#define ARG_OUTPUT     30

int getArgType(int argType);
int getArgLength(int argType);
int getSizeOfArgType(int type);

int numberOfArgs(int *argTypes);
int totalSizeOfArgs(int *argTypes);

bool isOnlyInput(int argType);
bool isOnlyOutput(int argType);
bool isInputOutput(int argType);

#endif
