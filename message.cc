#include "message.h"

#include <string>
#include <cstring>
#include <algorithm>

using namespace std;

MessageType extractMessageType(const char* buffer, int length) {
    if (length == 0) return MessageType::UNKNOWN;
    // This assumes the message is valid.
    return static_cast<MessageType>(buffer[0]);
}

char* copy_buffer(const char* buffer, int length) {
    char* dest = new char[length];
    memcpy(dest, buffer, length);
    return dest;
}

RawMessage::RawMessage(const char* buffer, int length)
    : buffer(copy_buffer(buffer, length)), length(length),
    message_type(extractMessageType(buffer, length)) {}

RawMessage::RawMessage() : RawMessage("", 0) {}

RawMessage::~RawMessage() {
    delete [] buffer;
}

RawMessage::RawMessage(const RawMessage& other)
    : buffer(copy_buffer(other.buffer, other.length)), length(other.length),
      message_type(other.message_type) {}

RawMessage& RawMessage::operator=(const RawMessage& other) {
    if (this == &other) return *this;
    RawMessage tmp = other;
    swap(buffer, tmp.buffer);
    swap(length, tmp.length);
    swap(message_type, tmp.message_type);
    return *this;
}

MessageType RawMessage::getMessageType() const {
    return message_type;
}

char RawMessage::getByte(int pos) const {
    //assert(0 <= pos && pos < length);
    return buffer[pos];
}

string RawMessage::getString() const {
    return std::string(buffer, length);
}
