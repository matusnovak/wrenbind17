#pragma once

#include "call.hpp"

namespace wrenbind17 {
    class Method {
    public:
        Method() : vm(nullptr) {
        }
        Method(WrenVM* vm, const std::shared_ptr<Handle>& variable, const std::shared_ptr<Handle>& handle)
            : vm(vm), variable(variable), handle(handle) {
        }
        ~Method() {
            reset();
        }

        template <typename... Args>
        ReturnValue operator()(Args&&... args) {
            return CallAndReturn<Args...>::func(vm, variable->getHandle(), handle->getHandle(),
                                                std::forward<Args>(args)...);
        }

        operator bool() const {
            return vm && variable && handle;
        }

        void reset() {
            vm = nullptr;
            handle.reset();
            variable.reset();
        }

    private:
        WrenVM* vm;
        std::shared_ptr<Handle> variable;
        std::shared_ptr<Handle> handle;
    };
} // namespace wrenbind17
