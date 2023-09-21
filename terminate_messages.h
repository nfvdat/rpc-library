#ifndef TERMINATE_MESSAGES_H
#define TERMINATE_MESSAGES_H

#include "message.h"
#include "protocol_macros.h"

// For convenience, define [Terminate] with a status too. Just don't use it.
MAKE_STATUS_MESSAGE(Terminate, MessageType::TERMINATE);
MAKE_STATUS_MESSAGE(TerminateSuccess, MessageType::TERMINATE_SUCCESS);
MAKE_STATUS_MESSAGE(TerminateFailure, MessageType::TERMINATE_FAILURE);

#endif
