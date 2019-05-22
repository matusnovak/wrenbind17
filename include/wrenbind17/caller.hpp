#pragma once

#include "slots.hpp"
#include "index.hpp"

namespace wrenbind17 {
    template <typename R, typename T, typename... Args>
    struct ForeignMethodCaller {
        template <R (T::*Fn)(Args...), size_t... Is>
        static void callFrom(WrenVM* vm, detail::index_list<Is...>) {
            auto self = PopHelper<T*>::f(vm, 0);
            auto ret = (self->*Fn)(PopHelper<typename std::remove_const<Args>::type>::f(vm, Is + 1)...);
            push<R>(vm, 0, ret);
        }

        template <R (T::*Fn)(Args...)>
        static void call(WrenVM* vm) {
            try {
                callFrom<Fn>(vm, detail::index_range<0, sizeof...(Args)>());
            } catch (...) {
                exceptionHandler(vm, std::current_exception());
            }
        }
    };

    template <typename T, typename... Args>
    struct ForeignMethodCaller<void, T, Args...> {
        template <void (T::*Fn)(Args...), size_t... Is>
        static void callFrom(WrenVM* vm, detail::index_list<Is...>) {
            auto self = PopHelper<T*>::f(vm, 0);
            (self->*Fn)(PopHelper<typename std::remove_const<Args>::type>::f(vm, Is + 1)...);
        }

        template <void (T::*Fn)(Args...)>
        static void call(WrenVM* vm) {
            try {
                callFrom<Fn>(vm, detail::index_range<0, sizeof...(Args)>());
            } catch (...) {
                exceptionHandler(vm, std::current_exception());
            }
        }
    };

    template <typename R, typename... Args>
    struct ForeignFunctionCaller {
        template <R (*Fn)(Args...), size_t... Is>
        static void callFrom(WrenVM* vm, detail::index_list<Is...>) {
            // auto self = pop<T*>(vm, 0);
            auto ret = (*Fn)(PopHelper<typename std::remove_const<Args>::type>::f(vm, Is + 1)...);
            push<R>(vm, 0, ret);
        }

        template <R (*Fn)(Args...)>
        static void call(WrenVM* vm) {
            try {
                callFrom<Fn>(vm, detail::index_range<0, sizeof...(Args)>());
            } catch (...) {
                exceptionHandler(vm, std::current_exception());
            }
        }
    };

    template <typename... Args>
    struct ForeignFunctionCaller<void, Args...> {
        template <void (*Fn)(Args...), size_t... Is>
        static void callFrom(WrenVM* vm, detail::index_list<Is...>) {
            // auto self = pop<T*>(vm, 0);
            (*Fn)(PopHelper<typename std::remove_const<Args>::type>::f(vm, Is + 1)...);
        }

        template <void (*Fn)(Args...)>
        static void call(WrenVM* vm) {
            try {
                callFrom<Fn>(vm, detail::index_range<0, sizeof...(Args)>());
            } catch (...) {
                exceptionHandler(vm, std::current_exception());
            }
        }
    };
} // namespace wrenbind17
