#pragma once

#include "slots.hpp"
#include "index.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    namespace detail {
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

            template <R (T::*Fn)(Args...) const, size_t... Is>
            static void callFrom(WrenVM* vm, detail::index_list<Is...>) {
                auto self = PopHelper<T*>::f(vm, 0);
                auto ret = (self->*Fn)(PopHelper<typename std::remove_const<Args>::type>::f(vm, Is + 1)...);
                push<R>(vm, 0, ret);
            }

            template <R (T::*Fn)(Args...) const>
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

            template <void (T::*Fn)(Args...) const, size_t... Is>
            static void callFrom(WrenVM* vm, detail::index_list<Is...>) {
                auto self = PopHelper<T*>::f(vm, 0);
                (self->*Fn)(PopHelper<typename std::remove_const<Args>::type>::f(vm, Is + 1)...);
            }

            template <void (T::*Fn)(Args...) const>
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

        template <typename T, typename V, V T::*Ptr>
        struct ForeignPropCaller {
            static void setter(WrenVM* vm) {
                auto self = PopHelper<T*>::f(vm, 0);
                self->*Ptr = PopHelper<V>::f(vm, 1);
            }
            static void getter(WrenVM* vm) {
                auto self = PopHelper<T*>::f(vm, 0);
                push<V>(vm, 0, self->*Ptr);
            }
        };
    } // namespace detail
} // namespace wrenbind17
