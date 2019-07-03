#pragma once

#include "handle.hpp"
#include "call.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    /**
     * @ingroup wrenbind17
     */
    class Callback {
    public:
        Callback() : vm(nullptr) {
        }

        Callback(Handle variable, Handle handle) : variable(std::move(variable)), handle(std::move(handle)) {
            vm = this->variable.vm;
        }

        Callback(const Callback& other) = delete;

        Callback(Callback&& other) noexcept : vm(nullptr) {
            swap(other);
        }

        Callback& operator=(const Callback& other) = delete;

        Callback& operator=(Callback&& other) noexcept {
            if (this != &other) {
                swap(other);
            }
            return *this;
        }

        void swap(Callback& other) noexcept {
            std::swap(vm, other.vm);
            variable.swap(other.variable);
            handle.swap(other.handle);
        }

        template <typename... Args> ReturnValue operator()(Args&&... args) {
            if (!vm)
                throw Exception("Bad callback instance");
            if (!handle)
                throw Exception("Callback's instance is null");
            if (!variable)
                throw Exception("Callback's function is null");

            return detail::CallAndReturn<Args...>::func(vm, variable.getHandle(), handle.getHandle(),
                                                        std::forward<Args>(args)...);
        }

        operator bool() const {
            return vm && handle && variable;
        }

        void reset() {
            vm = nullptr;
            handle.reset();
            variable.reset();
        }

    private:
        WrenVM* vm;
        Handle variable;
        Handle handle;
    };
}