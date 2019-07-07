#pragma once

#include "any.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    namespace detail {
        inline void pushArgs(WrenVM* vm, int idx) {
            (void)vm;
            (void)idx;
        }

        template <typename First, typename... Other> inline void pushArgs(
            WrenVM* vm, int idx, First&& first, Other&&... other) {
            PushHelper<First>::f(vm, idx, first);
            pushArgs(vm, ++idx, std::forward<Other>(other)...);
        }

        template <typename... Args> struct CallAndReturn {
            static Any func(WrenVM* vm, WrenHandle* handle, WrenHandle* func, Args&&... args) {
                constexpr auto n = sizeof...(Args);
                wrenEnsureSlots(vm, n + 1);
                wrenSetSlotHandle(vm, 0, handle);

                pushArgs(vm, 1, std::forward<Args>(args)...);

                if (wrenCall(vm, func) != WREN_RESULT_SUCCESS) {
                    getLastError(vm);
                }

                return getSlot<Any>(vm, 0);
            }
        };
    } // namespace detail

    /**
     * @ingroup wrenbind17
     */
    class Method {
    public:
        Method()
            : vm(nullptr) {
        }

        Method(WrenVM* vm, std::shared_ptr<Handle> variable, std::shared_ptr<Handle> handle)
            : vm(vm),
              variable(std::move(variable)),
              handle(std::move(handle)) {
        }

        ~Method() {
            reset();
        }

        template <typename... Args> Any operator()(Args&&... args) {
            return detail::CallAndReturn<Args...>::func(vm, variable->getHandle(), handle->getHandle(),
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
