#ifndef PROTOCOL_MACROS_H
#define PROTOCOL_MACROS_H

#include <string>

#define GET_BYTE(x, i) (((x) >> ((i) << 3)) & 0xff)
#define MAKE_BYTE(x, i) (((int) ((unsigned char) (x))) << ((i) << 3))

std::string encode_integer(int x);

/* In these functions, [buffer] at [next_pos] is assumed to contain
 * the desired field. The value of [next_pos] will be updated to advance
 * to the next field.
 */
int decode_integer(const std::string& buffer, int* next_pos);
std::string decode_string(const std::string& buffer, int* next_pos);

/* Make a simple message type with a simple integer [status]
 * field and serialization.
 *
 * [Name] is the name of the message struct.
 * [Type] is the [MessageType] of the message.
 */
#define MAKE_STATUS_MESSAGE(Name, Type) \
    struct Name { \
        const int status; /* 0 or warnings if any */ \
    }; \
    template<> class Serializer<Name> { \
    public: \
        static RawMessage serialize(const Name& message) { \
            const int length = 2; \
            std::string buffer(length, ' '); \
            buffer[0] = static_cast<char>(Type); \
            buffer[1] = static_cast<char>(message.status); \
     \
            return RawMessage(buffer.c_str(), length); \
        } \
     \
        static Name* deserialize(const RawMessage& raw_message) { \
            /*assert(raw_message.getMessageType() == Type);*/ \
            int status = raw_message.getByte(1); \
            return new Name{ status }; \
        } \
    }; \

#endif
