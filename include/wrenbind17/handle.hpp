#pragma once

#include <wren.hpp>
#include <memory>

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    class Callback;
    /**
     * @ingroup wrenbind17
     */
    class Handle {
    public:
        Handle() : vm(nullptr), handle(nullptr) {
        }
        Handle(WrenVM* vm, WrenHandle* handle) : vm(vm), handle(handle) {
        }
        ~Handle() {
            reset();
        }
        Handle(const Handle& other) = delete;
        Handle(Handle&& other) noexcept : vm(nullptr), handle(nullptr) {
            swap(other);
        }
        Handle& operator=(const Handle& other) = delete;
        Handle& operator=(Handle&& other) noexcept {
            if (this != &other) {
                swap(other);
            }
            return *this;
        }
        void swap(Handle& other) noexcept {
            std::swap(vm, other.vm);
            std::swap(handle, other.handle);
        }

        WrenHandle* getHandle() const {
            return handle;
        }

        WrenVM* getVm() const {
            return vm;
        }

        void reset() {
            if (vm && handle) {
                wrenReleaseHandle(vm, handle);
                vm = nullptr;
                handle = nullptr;
            }
        }

        operator bool() const {
            return vm && handle;
        }

        friend Callback;
    private:
        WrenVM* vm;
        WrenHandle* handle;
    };
} // namespace wrenbind17
