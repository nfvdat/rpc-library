#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <vector>
#include <tuple>
#include <utility>
#include <string>

/* A type for the signature of a skeleton, that is, its name and the
 * (ordered) types of its arguments. Ignores lengths of arrays, so that
 * two RPCs should be considered the same if and only if this type has
 * equality.
 */

enum class ScalarOrArray {
    SCALAR,
    ARRAY
};

enum class InOutBoth {
    INPUT,
    OUTPUT,
    BOTH
};

enum class DataType {
    CHAR,
    SHORT,
    INT,
    LONG,
    DOUBLE,
    FLOAT
};

typedef std::tuple<ScalarOrArray, InOutBoth, DataType> Argument;
typedef std::vector<Argument> ArgumentList;
typedef std::pair<std::string, ArgumentList> Signature;

// Converts assignment spec to signature.
Signature signature_of_arg_types(std::string function_name, int* arg_types);

#endif
