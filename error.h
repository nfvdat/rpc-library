#ifndef ERROR_H
#define ERROR_H

#include <string>

/* A simple base classes for errors that encapsulates a reason.
 * Derive from it to throw specific error objects that we can match against
 * in [catch] clauses.
 */
class Error {
private:
    const std::string reason;

public:
    Error(const std::string& reason = "");
    std::string getReason() const;
};

// Use this to avoid boilerplate.
#define MAKE_ERROR_CLASS(name) \
    class name : public Error { \
        using Error::Error; \
    };

#endif
