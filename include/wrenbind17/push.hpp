#pragma once

#include "object.hpp"
#include <list>
#include <queue>
#include <vector>

namespace wrenbind17 {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    void getClassType(WrenVM* vm, std::string& module, std::string& name, size_t hash);
    bool isClassRegistered(WrenVM* vm, const size_t hash);
    detail::ForeignPtrConvertor* getClassCast(WrenVM* vm, size_t hash, size_t other);

    namespace detail {
        template <typename T> struct PushHelper;

        template <typename T> void pushAsConstRef(WrenVM* vm, int idx, const T& value) {
            static_assert(!std::is_same<int, typename std::remove_const<T>::type>(), "type can't be int");
            static_assert(!std::is_same<std::string, typename std::remove_const<T>::type>(),
                          "type can't be std::string");
            static_assert(!is_shared_ptr<T>::value, "type can't be shared_ptr<T>");
            try {
                std::string module;
                std::string klass;
                getClassType(vm, module, klass, typeid(T).hash_code());

                wrenEnsureSlots(vm, idx + 1);
                wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<T>));
                auto* foreign = new (memory) ForeignObject<T>(std::make_shared<T>(value));
                (void)foreign;
            } catch (std::out_of_range& e) {
                (void)e;
                throw BadCast("Class type not registered in Wren VM");
            }
        }

        template <typename T> void pushAsMove(WrenVM* vm, int idx, T&& value) {
            static_assert(!std::is_same<int, T>(), "type can't be int");
            static_assert(!std::is_same<std::string, T>(), "type can't be std::string");
            static_assert(!is_shared_ptr<T>::value, "type can't be shared_ptr<T>");
            try {
                std::string module;
                std::string klass;
                getClassType(vm, module, klass, typeid(T).hash_code());

                wrenEnsureSlots(vm, idx + 1);
                wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<T>));
                auto* foreign = new (memory) ForeignObject<T>(std::make_shared<T>(std::move(value)));
                (void)foreign;
            } catch (std::out_of_range& e) {
                (void)e;
                throw BadCast("Class type not registered in Wren VM");
            }
        }

        template <typename T> void pushAsPtr(WrenVM* vm, int idx, T* value) {
            static_assert(!std::is_same<int, T>(), "type can't be int");
            static_assert(!std::is_same<std::string, T>(), "type can't be std::string");
            static_assert(!is_shared_ptr<T>::value, "type can't be shared_ptr<T>");
            try {
                std::string module;
                std::string klass;
                getClassType(vm, module, klass, typeid(T).hash_code());

                wrenEnsureSlots(vm, idx + 1);
                wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<T>));
                auto* foreign = new (memory) ForeignObject<T>(std::shared_ptr<T>(value, [](T* t) {}));
                (void)foreign;
            } catch (std::out_of_range& e) {
                (void)e;
                throw BadCast("Class type not registered in Wren VM");
            }
        }

        template <typename T> struct PushHelper {
            static inline void f(WrenVM* vm, int idx, const T& value) {
                pushAsConstRef(vm, idx, value);
            }

            static inline void f(WrenVM* vm, int idx, T&& value) {
                pushAsMove(vm, idx, std::move(value));
            }
        };

        template <typename T> struct PushHelper<const T> {
            static inline void f(WrenVM* vm, int idx, const T value) {
                PushHelper<T*>::f(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<T*> {
            static inline void f(WrenVM* vm, int idx, T* value) {
                pushAsPtr(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<const T*> {
            static inline void f(WrenVM* vm, int idx, const T* value) {
                PushHelper<T*>::f(vm, idx, const_cast<T*>(value));
            }
        };

        template <typename T> struct PushHelper<const T*&> {
            static inline void f(WrenVM* vm, int idx, const T*& value) {
                PushHelper<T*>::f(vm, idx, const_cast<T*>(value));
            }
        };

        template <typename T> struct PushHelper<T&> {
            static inline void f(WrenVM* vm, int idx, T& value) {
                PushHelper<T>::f(vm, idx, static_cast<const T&>(value));
            }
        };

        template <typename T> struct PushHelper<const T&> {
            static inline void f(WrenVM* vm, int idx, const T& value) {
                PushHelper<T>::f(vm, idx, static_cast<const T&>(value));
            }
        };

        // ============================================================================================================
        //                                       BASIC TYPES
        // ============================================================================================================

#define WRENBIND17_PUSH_HELPER(T, FUNC)                                                                                \
    template <> inline void PushHelper<T>::f(WrenVM* vm, int idx, const T& value) {                                    \
        FUNC;                                                                                                          \
    }                                                                                                                  \
    template <> inline void PushHelper<T>::f(WrenVM* vm, int idx, T&& value) {                                         \
        FUNC;                                                                                                          \
    }                                                                                                                  \
    template <> inline void PushHelper<const T>::f(WrenVM* vm, int idx, const T value) {                               \
        FUNC;                                                                                                          \
    }                                                                                                                  \
    template <> inline void PushHelper<T&>::f(WrenVM* vm, int idx, T& value) {                                         \
        FUNC;                                                                                                          \
    }                                                                                                                  \
    template <> inline void PushHelper<const T&>::f(WrenVM* vm, int idx, const T& value) {                             \
        FUNC;                                                                                                          \
    }                                                                                                                  \
    template <> inline void PushHelper<T*>::f(WrenVM* vm, int idx, T* value) {                                         \
        PushHelper<T>::f(vm, idx, *value);                                                                             \
    }                                                                                                                  \
    template <> inline void PushHelper<const T*>::f(WrenVM* vm, int idx, const T* value) {                             \
        PushHelper<T>::f(vm, idx, *value);                                                                             \
    }

        WRENBIND17_PUSH_HELPER(char, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(int8_t, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(short, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(int, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(long, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(long long, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(uint8_t, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(unsigned short, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(unsigned int, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(unsigned long, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(unsigned long long, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(float, wrenSetSlotDouble(vm, idx, static_cast<double>(value)));
        WRENBIND17_PUSH_HELPER(double, wrenSetSlotDouble(vm, idx, value));
        WRENBIND17_PUSH_HELPER(bool, wrenSetSlotBool(vm, idx, value));
        WRENBIND17_PUSH_HELPER(std::nullptr_t, wrenSetSlotNull(vm, idx));

        // ============================================================================================================
        //                                       STD STRING
        // ============================================================================================================

        template <> struct PushHelper<std::string> {
            static inline void f(WrenVM* vm, int idx, const std::string value) {
                wrenSetSlotString(vm, idx, value.c_str());
            }
        };

        template <> struct PushHelper<const std::string> {
            static inline void f(WrenVM* vm, int idx, const std::string value) {
                wrenSetSlotString(vm, idx, value.c_str());
            }
        };

        template <> struct PushHelper<std::string&&> {
            static inline void f(WrenVM* vm, int idx, std::string&& value) {
                wrenSetSlotString(vm, idx, value.c_str());
            }
        };

        template <> struct PushHelper<std::string&> {
            static inline void f(WrenVM* vm, int idx, std::string& value) {
                wrenSetSlotString(vm, idx, value.c_str());
            }
        };

        template <> struct PushHelper<const std::string&> {
            static inline void f(WrenVM* vm, int idx, const std::string& value) {
                wrenSetSlotString(vm, idx, value.c_str());
            }
        };

        // ============================================================================================================
        //                                       STD SHARED POINTERS
        // ============================================================================================================

        template <typename T> struct PushHelper<std::shared_ptr<T>> {
            static inline void f(WrenVM* vm, int idx, std::shared_ptr<T> value) {
                static_assert(!std::is_same<std::string, T>(), "type can't be std::string");
                static_assert(!is_shared_ptr<T>::value, "type can't be shared_ptr<T>");
                try {
                    std::string module;
                    std::string klass;
                    getClassType(vm, module, klass, typeid(T).hash_code());

                    wrenEnsureSlots(vm, idx + 1);
                    wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                    auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<T>));
                    auto* foreign = new (memory) ForeignObject<T>(value);
                    (void)foreign;
                } catch (std::out_of_range& e) {
                    (void)e;
                    throw BadCast("Class type not registered in Wren VM");
                }
            }
        };

        template <typename T> struct PushHelper<const std::shared_ptr<T>> {
            static inline void f(WrenVM* vm, int idx, const std::shared_ptr<T> value) {
                PushHelper<std::shared_ptr<T>>::f(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<std::shared_ptr<T>&&> {
            static inline void f(WrenVM* vm, int idx, std::shared_ptr<T>&& value) {
                PushHelper<std::shared_ptr<T>>::f(vm, idx, std::move(value));
            }
        };

        template <typename T> struct PushHelper<std::shared_ptr<T>&> {
            static inline void f(WrenVM* vm, int idx, std::shared_ptr<T>& value) {
                PushHelper<std::shared_ptr<T>>::f(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<const std::shared_ptr<T>&> {
            static inline void f(WrenVM* vm, int idx, const std::shared_ptr<T>& value) {
                PushHelper<std::shared_ptr<T>>::f(vm, idx, value);
            }
        };

        // ============================================================================================================
        //                                       STD VARIANT
        // ============================================================================================================

        template <typename VariantType>
        inline void loopAndPushVariant(WrenVM* vm, int idx, const VariantType& v, size_t i) {
            PushHelper<std::nullptr_t>::f(vm, idx, nullptr);
        }

        template <typename VariantType, typename T, typename... Ts>
        inline void loopAndPushVariant(WrenVM* vm, int idx, const VariantType& v, size_t i) {
            if (v.index() == i) {
                PushHelper<T>::f(vm, idx, std::get<T>(v));
            } else {
                loopAndPushVariant<VariantType, Ts...>(vm, idx, v, i + 1);
            }
        }

        template <typename... Ts> struct PushHelper<std::variant<Ts...>> {
            inline static void f(WrenVM* vm, int idx, const std::variant<Ts...>& value) {
                loopAndPushVariant<std::variant<Ts...>, Ts...>(vm, idx, value, 0);
            }
        };

        template <typename... Ts> struct PushHelper<std::variant<Ts...>&> {
            inline static void f(WrenVM* vm, int idx, const std::variant<Ts...>& value) {
                PushHelper<std::variant<Ts...>>::f(vm, idx, value);
            }
        };

        template <typename... Ts> struct PushHelper<std::variant<Ts...>*> {
            inline static void f(WrenVM* vm, int idx, const std::variant<Ts...>* value) {
                PushHelper<std::variant<Ts...>>::f(vm, idx, *value);
            }
        };

        template <typename... Ts> struct PushHelper<const std::variant<Ts...>&> {
            inline static void f(WrenVM* vm, int idx, const std::variant<Ts...>& value) {
                PushHelper<std::variant<Ts...>>::f(vm, idx, value);
            }
        };

        // ============================================================================================================
        //                                       STD VECTOR
        // ============================================================================================================

        template <typename Iter> inline void loopAndPushIterable(WrenVM* vm, const int idx, Iter begin, Iter end) {
            using T = typename std::iterator_traits<Iter>::value_type;
            wrenSetSlotNewList(vm, idx);
            auto i = 0;
            for (auto it = begin; it != end; ++it) {
                PushHelper<T>::f(vm, idx + 1, *it);
                wrenInsertInList(vm, idx, i++, idx + 1);
            }
        }

        template <typename T> struct PushHelper<std::vector<T>> {
            static inline void f(WrenVM* vm, int idx, std::vector<T> value) {
                if (isClassRegistered(vm, typeid(std::vector<T>).hash_code())) {
                    pushAsMove<std::vector<T>>(vm, idx, std::move(value));
                } else {
                    loopAndPushIterable(vm, idx, value.begin(), value.end());
                }
            }
        };

        template <typename T> struct PushHelper<std::vector<T>*> {
            static inline void f(WrenVM* vm, int idx, std::vector<T>* value) {
                if (isClassRegistered(vm, typeid(std::vector<T>).hash_code())) {
                    pushAsPtr<std::vector<T>>(vm, idx, value);
                } else {
                    loopAndPushIterable(vm, idx, value->begin(), value->end());
                }
            }
        };

        template <typename T> struct PushHelper<const std::vector<T>&> {
            static inline void f(WrenVM* vm, int idx, const std::vector<T>& value) {
                if (isClassRegistered(vm, typeid(std::vector<T>).hash_code())) {
                    pushAsConstRef<std::vector<T>>(vm, idx, value);
                } else {
                    loopAndPushIterable(vm, idx, value.begin(), value.end());
                }
            }
        };

        // ============================================================================================================
        //                                       STD List
        // ============================================================================================================

        template <typename T> struct PushHelper<std::list<T>> {
            static inline void f(WrenVM* vm, int idx, std::list<T> value) {
                if (isClassRegistered(vm, typeid(std::list<T>).hash_code())) {
                    pushAsMove<std::list<T>>(vm, idx, std::move(value));
                } else {
                    loopAndPushIterable(vm, idx, value.begin(), value.end());
                }
            }
        };

        template <typename T> struct PushHelper<std::list<T>*> {
            static inline void f(WrenVM* vm, int idx, std::list<T>* value) {
                if (isClassRegistered(vm, typeid(std::list<T>).hash_code())) {
                    pushAsPtr<std::list<T>>(vm, idx, value);
                } else {
                    loopAndPushIterable(vm, idx, value->begin(), value->end());
                }
            }
        };

        template <typename T> struct PushHelper<const std::list<T>&> {
            static inline void f(WrenVM* vm, int idx, const std::list<T>& value) {
                if (isClassRegistered(vm, typeid(std::list<T>).hash_code())) {
                    pushAsConstRef<std::list<T>>(vm, idx, value);
                } else {
                    loopAndPushIterable(vm, idx, value.begin(), value.end());
                }
            }
        };

        // ============================================================================================================
        //                                       STD Queue
        // ============================================================================================================

        template <typename T> struct PushHelper<std::queue<T>> {
            static inline void f(WrenVM* vm, int idx, std::queue<T> value) {
                if (isClassRegistered(vm, typeid(std::queue<T>).hash_code())) {
                    pushAsMove<std::queue<T>>(vm, idx, std::move(value));
                } else {
                    loopAndPushIterable(vm, idx, value.begin(), value.end());
                }
            }
        };

        template <typename T> struct PushHelper<std::queue<T>*> {
            static inline void f(WrenVM* vm, int idx, std::queue<T>* value) {
                if (isClassRegistered(vm, typeid(std::queue<T>).hash_code())) {
                    pushAsPtr<std::queue<T>>(vm, idx, value);
                } else {
                    loopAndPushIterable(vm, idx, value->begin(), value->end());
                }
            }
        };

        template <typename T> struct PushHelper<const std::queue<T>&> {
            static inline void f(WrenVM* vm, int idx, const std::queue<T>& value) {
                if (isClassRegistered(vm, typeid(std::queue<T>).hash_code())) {
                    pushAsConstRef<std::queue<T>>(vm, idx, value);
                } else {
                    loopAndPushIterable(vm, idx, value.begin(), value.end());
                }
            }
        };
    } // namespace detail
#endif
} // namespace wrenbind17
