#include "signature.h"

#include "arg.h"

using namespace std;

Signature signature_of_arg_types(string function_name, int* arg_types) {
    int num_args = numberOfArgs(arg_types);
    vector<Argument> arg_list;
    for (int i = 0; i < num_args; i++) {
        DataType data_type;
        switch (getArgType(arg_types[i])) {
            case ARG_CHAR:
                data_type = DataType::CHAR;
                break;
            case ARG_SHORT:
                data_type = DataType::SHORT;
                break;
            case ARG_INT:
                data_type = DataType::INT;
                break;
            case ARG_LONG:
                data_type = DataType::LONG;
                break;
            case ARG_DOUBLE:
                data_type = DataType::DOUBLE;
                break;
            case ARG_FLOAT:
                data_type = DataType::FLOAT;
                break;
            default:
                //assert(false && "Unknown data type");
                ;
        }

        // Do it manually to distinguish between array of size 1 and scalar.
        int length = (arg_types[i] & ((1<<16)-1));
        ScalarOrArray dimension;
        if (length == 0)
            dimension = ScalarOrArray::SCALAR;
        else
            dimension = ScalarOrArray::ARRAY;

        InOutBoth direction;
        if (isOnlyInput(arg_types[i]))
            direction = InOutBoth::INPUT;
        else if (isOnlyOutput(arg_types[i]))
            direction = InOutBoth::OUTPUT;
        else if (isInputOutput(arg_types[i]))
            direction = InOutBoth::BOTH;
        //else
            //assert(false && "No in/out bit specified");

        arg_list.push_back(make_tuple(dimension, direction, data_type));
    }

    return make_pair(function_name, arg_list);
}
