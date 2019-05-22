#pragma once

#include <stdexcept>

namespace wrenbind17 {
    class Exception : public std::exception {
    public:
        Exception() = default;

        explicit Exception(std::string msg):msg(std::move(msg)) {
        }

        const char *what() const throw() override {
            return msg.c_str();
        }

    private:
        std::string msg;
    };

    class NotFound : public Exception {
    public:
        NotFound(): Exception("Not found"){
        }
    };

    class BadCast : public std::bad_cast {
    public:
        BadCast() = default;
    };
} // namespace wrenbind17
