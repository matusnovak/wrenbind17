#pragma once

#include "method.hpp"
#include "slots.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    /**
     * @ingroup wrenbind17
     */
    class Variable {
    public:
        Variable() : vm(nullptr) {
        }
        Variable(WrenVM* vm, const std::shared_ptr<Handle>& handle) : vm(vm), handle(handle) {
        }
        ~Variable() {
            reset();
        }

        Method func(const std::string& signature) {
            auto* h = wrenMakeCallHandle(vm, signature.c_str());
            return Method(vm, handle, std::make_shared<Handle>(vm, h));
        }

        WrenHandle* getHandle() const {
            return handle->getHandle();
        }

        operator bool() const {
            return vm && handle;
        }

        void reset() {
            vm = nullptr;
            handle.reset();
        }

    private:
        WrenVM* vm;
        std::shared_ptr<Handle> handle;
    };

    template <>
    inline Variable detail::getSlot(WrenVM* vm, int idx) {
        validate<WrenType::WREN_TYPE_UNKNOWN>(vm, idx);
        return Variable(vm, std::make_shared<Handle>(vm, wrenGetSlotHandle(vm, idx)));
    }
} // namespace wrenbind17
