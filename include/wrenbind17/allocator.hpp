#pragma once

#include "slots.hpp"
#include "index.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    namespace detail {
        template <typename T, typename... Args>
        struct ForeignKlassAllocator {
            static T* ctor(Args&&... args) {
                return new T(std::forward<Args>(args)...);
            }
            template <size_t... Is>
            static T* ctorFrom(WrenVM* vm, detail::index_list<Is...>) {
                return ctor(PopHelper<Args>::f(vm, Is + 1)...);
            }
            static void allocate(WrenVM* vm) {
                auto* memory = wrenSetSlotNewForeign(vm, 0, 0, sizeof(ForeignObject<T>));
                auto* ptr = ctorFrom(vm, detail::index_range<0, sizeof...(Args)>());
                new (memory) ForeignObject<T>(std::shared_ptr<T>(ptr));
            }
            static void finalize(void* memory) {
                auto* wrapper = reinterpret_cast<ForeignObject<T>*>(memory);
                wrapper->~ForeignObject<T>();
            }
        };
    } // namespace detail
} // namespace wrenbind17
