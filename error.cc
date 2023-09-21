#include "error.h"

#include <string>

using namespace std;

Error::Error(const string& reason) : reason(reason) {}

string Error::getReason() const {
    return reason;
}
