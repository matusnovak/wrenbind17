#pragma once

#include "object.hpp"

namespace wrenbind17 {
    void getClassType(WrenVM* vm, std::string& module, std::string& name, size_t hash);
    detail::ForeignPtrConvertor* getClassCast(WrenVM* vm, size_t hash, size_t other);

    namespace detail {
        // ============================================================================================================
        //                                       BASIC TYPES
        // ============================================================================================================
        template <typename T> inline void setSlot(WrenVM* vm, int idx, T&& value) {
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
            } catch (std::out_of_range& e) {
                (void)e;
                throw BadCast("Class type not registered in Wren VM");
            }
        }

        template <typename T> inline void setSlot(WrenVM* vm, int idx, const T& value) {
            static_assert(!std::is_same<std::string, T>(), "type can't be std::string");
            static_assert(!is_shared_ptr<T>::value, "type can't be shared_ptr<T>");
            try {
                std::string module;
                std::string klass;
                getClassType(vm, module, klass, typeid(T).hash_code());

                wrenEnsureSlots(vm, idx + 1);
                wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<T>));
                auto* foreign = new (memory) ForeignObject<T>(std::make_shared<T>(value));
            } catch (std::out_of_range& e) {
                (void)e;
                throw BadCast("Class type not registered in Wren VM");
            }
        }

        template <typename T> struct PushHelper {
            inline static void f(WrenVM* vm, int idx, const T& value) {
                static_assert(!std::is_pointer<T>::value, "type can't be a pointer");
                setSlot<T>(vm, idx, value);
            }
            inline static void f(WrenVM* vm, int idx, T&& value) {
                static_assert(!std::is_pointer<T>::value, "type can't be a pointer");
                setSlot<T>(vm, idx, std::move(value));
            }
        };

        template <>
        inline void PushHelper<std::string>::f(WrenVM* vm, int idx, std::string&& value) {
            wrenSetSlotString(vm, idx, value.c_str());
        }

        template <>
        inline void PushHelper<std::nullptr_t>::f(WrenVM* vm, int idx, std::nullptr_t&& value) {
            wrenSetSlotNull(vm, idx);
        }

        template <typename T> struct PushHelper<const T> {
            inline static void f(WrenVM* vm, int idx, T value) {
                static_assert(!std::is_pointer<T>::value, "type can't be a pointer");
                setSlot<T>(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<const T&> {
            inline static void f(WrenVM* vm, int idx, const T& value) {
                setSlot<typename std::remove_const<typename std::remove_reference<T>::type>::type>(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<T*> {
            inline static void f(WrenVM* vm, int idx, T* value) {
                using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
                try {
                    std::string module;
                    std::string klass;
                    getClassType(vm, module, klass, typeid(Type).hash_code());

                    wrenEnsureSlots(vm, idx + 1);
                    wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                    auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<Type>));
                    auto* foreign = new (memory)
                        ForeignObject<Type>(std::shared_ptr<Type>(const_cast<Type*>(value), [](Type* t) {}));
                } catch (std::out_of_range& e) {
                    (void)e;
                    throw BadCast("Class type not registered in Wren VM");
                }
            }
        };

        template <typename T> struct PushHelper<T&> {
            inline static void f(WrenVM* vm, int idx, T& value) {
                setSlot<T>(vm, idx, value);
            }

            inline static void f(WrenVM* vm, int idx, const T& value) {
                setSlot<T>(vm, idx, value);
            }

            inline static void f(WrenVM* vm, int idx, T&& value) {
                setSlot<typename std::remove_const<typename std::remove_reference<T>::type>::type>(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<const T*> {
            inline static void f(WrenVM* vm, int idx, const T* value) {
                PushHelper<T*>::f(vm, idx, const_cast<T*>(value));
            }
        };

        template <typename T> struct PushHelper<const T*&> {
            inline static void f(WrenVM* vm, int idx, const T*& value) {
                PushHelper<T*>::f(vm, idx, const_cast<T*>(value));
            }
        };

        template <typename T> struct PushHelper<std::shared_ptr<T>> {
            inline static void f(WrenVM* vm, int idx, const std::shared_ptr<T>& value) {
                try {
                    std::string module;
                    std::string klass;
                    getClassType(vm, module, klass, typeid(T).hash_code());

                    wrenEnsureSlots(vm, idx + 1);
                    wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                    auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<T>));
                    auto* foreign = new (memory) ForeignObject<T>(value);
                } catch (std::out_of_range& e) {
                    (void)e;
                    throw BadCast("Class type not registered in Wren VM");
                }
            }
        };

        template <typename T> struct PushHelper<std::shared_ptr<T>&> {
            inline static void f(WrenVM* vm, int idx, std::shared_ptr<T>& value) {
                PushHelper<std::shared_ptr<T>>::f(vm, idx, value);
            }
        };

        template <typename T> struct PushHelper<const std::shared_ptr<T>&> {
            inline static void f(WrenVM* vm, int idx, const std::shared_ptr<T>& value) {
                PushHelper<std::shared_ptr<T>>::f(vm, idx, value);
            }
        };

        template <> struct PushHelper<const std::string&> {
            inline static void f(WrenVM* vm, int idx, const std::string& value) {
                wrenSetSlotString(vm, idx, value.c_str());
            }
        };

        template <> inline void setSlot(WrenVM* vm, int idx, const std::nullptr_t& value) {
            (void)value;
            wrenSetSlotNull(vm, idx);
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const std::string& value) {
            wrenSetSlotString(vm, idx, value.c_str());
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const bool& value) {
            wrenSetSlotBool(vm, idx, value);
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const int8_t& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const char& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const int& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const short& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const long& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const unsigned long& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const unsigned short& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const unsigned char& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const unsigned& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const long long& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const unsigned long long& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const float& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, const double& value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

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

        template <typename... Ts> struct PushHelper<const std::variant<Ts...>&> {
            inline static void f(WrenVM* vm, int idx, const std::variant<Ts...>& value) {
                PushHelper<std::variant<Ts...>>::f(vm, idx, value);
            }
        };
    } // namespace detail
} // namespace wrenbind17
