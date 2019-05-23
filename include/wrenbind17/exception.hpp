#pragma once

#include <stdexcept>

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    /**
     * @ingroup wrenbind17
     */
    class Exception : public std::exception {
    public:
        Exception() = default;

        explicit Exception(std::string msg) : msg(std::move(msg)) {
        }

        const char* what() const throw() override {
            return msg.c_str();
        }

    private:
        std::string msg;
    };

    /**
     * @ingroup wrenbind17
     */
    class NotFound : public Exception {
    public:
        NotFound() : Exception("Not found") {
        }
    };

    /**
     * @ingroup wrenbind17
     */
    class BadCast : public std::bad_cast {
    public:
        BadCast() = default;
    };
} // namespace wrenbind17
