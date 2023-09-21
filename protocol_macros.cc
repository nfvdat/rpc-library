#include "protocol_macros.h"

#include <iostream>

using namespace std;

string encode_integer(int x) {
    string buffer;

    buffer += static_cast<unsigned char>(GET_BYTE(x, 0));
    buffer += static_cast<unsigned char>(GET_BYTE(x, 1));
    buffer += static_cast<unsigned char>(GET_BYTE(x, 2));
    buffer += static_cast<unsigned char>(GET_BYTE(x, 3));
    return buffer;
}

int decode_integer(const string& buffer, int* next_pos) {
    *next_pos += 4;
    return
        MAKE_BYTE(buffer[*next_pos - 4], 0) +
        MAKE_BYTE(buffer[*next_pos - 3], 1) +
        MAKE_BYTE(buffer[*next_pos - 2], 2) +
        MAKE_BYTE(buffer[*next_pos - 1], 3);
}

string decode_string(const string& buffer, int* next_pos) {
    int length = decode_integer(buffer, next_pos);
    int old_pos = *next_pos;
    *next_pos += length;
    return buffer.substr(old_pos, length);
}
