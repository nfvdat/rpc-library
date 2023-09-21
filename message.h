#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

// Add error codes here so we can easily refer to and document them.
// Negative numbers for errors, positive for warnings.
// Big negative numbers so as to not conflict with skeleton error codes.
enum class StatusCodes {
    SERVER_SOCKET_CREATION_ERROR = -1001,
    BINDER_CONNECTION_ERROR      = -1002,
    UNEXPECTED_MESSAGE           = -1003,
    SERVER_CONNECTION_ERROR      = -1004,
    LOCATE_FAILURE               = -1005,
};

enum class MessageType {
    REGISTER = 0,
    REGISTER_SUCCESS,
    REGISTER_FAILURE,
    LOCATE_REQUEST,
    LOCATE_SUCCESS,
    LOCATE_FAILURE,
    EXECUTE,  // EXECUTE_SUCCESS is the same message type
    EXECUTE_FAILURE,
    TERMINATE,
    TERMINATE_SUCCESS,
    TERMINATE_FAILURE,
    UNKNOWN
};

/* Contains a raw message, that is, the bytes that are communicated to
 * nodes and interpreted as specific message types. */
class RawMessage {
private:
    char* buffer;
    int length;
    MessageType message_type;

public:
    // This makes a copy of the buffer, so it does not take
    // ownership (meaning the caller must still deal with
    // cleaning up its buffer).
    RawMessage(const char* buffer, int length);
    RawMessage();
    ~RawMessage();
    RawMessage(const RawMessage& other);
    RawMessage& operator=(const RawMessage& other);

    MessageType getMessageType() const;
    char getByte(int pos) const;
    std::string getString() const;
};

/* An interface for a serializer, that is, a "two-way" factory to serialize and
 * deserialize messages into/from objects of the real message type [MessageT].
 *
 * This must be implemented for each specific message type by specializing
 * the template.
 */
template <class MessageT>
class Serializer {
public:
    static RawMessage serialize(const MessageT& message);
    static MessageT* deserialize(const RawMessage& raw_message);
};

#endif
