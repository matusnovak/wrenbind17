#pragma once

#include <any>
#include <typeinfo>
#include "handle.hpp"
#include "slots.hpp"

namespace wrenbind17 {
    class ReturnValue {
    public:
        explicit ReturnValue(double value):type{WrenType::WREN_TYPE_NUM}, value{value} {
        };

        explicit ReturnValue(bool value):type{WrenType::WREN_TYPE_BOOL}, value{value} {
        };

        explicit ReturnValue(std::nullptr_t value):type{WrenType::WREN_TYPE_NULL}, value{value} {
        };

        explicit ReturnValue(std::string value):type{WrenType::WREN_TYPE_STRING}, value{std::move(value)} {
        };

        explicit ReturnValue(void* value):type{WrenType::WREN_TYPE_FOREIGN}, value{std::move(value)} {
        };

        template<typename T>
        bool is() const {
            if (type != WREN_TYPE_FOREIGN) return false;
            using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            auto foreign = reinterpret_cast<ForeignObject<Type>*>(std::any_cast<void*>(value));
            return foreign->hash() == typeid(Type).hash_code();
        }

        template <typename T> inline typename std::enable_if<!std::is_pointer<T>::value, T>::type
        as() {
            if (type != WREN_TYPE_FOREIGN) throw BadCast();
            using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            auto foreign = reinterpret_cast<ForeignObject<Type>*>(std::any_cast<void*>(value));
            if (foreign->hash() != typeid(Type).hash_code()) {
                throw BadCast();
            }
            return *reinterpret_cast<Type*>(foreign->get());
        }

        template <typename T> inline typename std::enable_if<std::is_pointer<T>::value, T>::type
        as() {
            if (type != WREN_TYPE_FOREIGN) throw BadCast();
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            auto foreign = reinterpret_cast<ForeignObject<Type>*>(std::any_cast<void*>(value));
            if (foreign->hash() != typeid(Type).hash_code()) {
                throw BadCast();
            }
            return reinterpret_cast<Type*>(foreign->get());
        }
    private:
        WrenType type;
        std::any value;
    };

    template<>
    inline bool ReturnValue::is<int8_t>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<char>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<short>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<int>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<long>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<long long>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<unsigned char>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<unsigned short>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<unsigned int>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<unsigned long>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<unsigned long long>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<float>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<double>() const {
        return type == WREN_TYPE_NUM;
    }

    template<>
    inline bool ReturnValue::is<bool>() const {
        return type == WREN_TYPE_BOOL;
    }

    template<>
    inline bool ReturnValue::is<std::nullptr_t>() const {
        return type == WREN_TYPE_NULL;
    }

    template<>
    inline bool ReturnValue::is<std::string>() const {
        return type == WREN_TYPE_STRING;
    }

    template<>
    inline int8_t ReturnValue::as<int8_t>() {
        return static_cast<int8_t>(std::any_cast<double>(value));
    }

    template<>
    inline char ReturnValue::as<char>() {
        return static_cast<char>(std::any_cast<double>(value));
    }

    template<>
    inline short ReturnValue::as<short>() {
        return static_cast<short>(std::any_cast<double>(value));
    }

    template<>
    inline int ReturnValue::as<int>() {
        return static_cast<int>(std::any_cast<double>(value));
    }

    template<>
    inline long ReturnValue::as<long>() {
        return static_cast<long>(std::any_cast<double>(value));
    }

    template<>
    inline long long ReturnValue::as<long long>() {
        return static_cast<long long>(std::any_cast<double>(value));
    }

    template<>
    inline unsigned char ReturnValue::as<unsigned char>() {
        return static_cast<char>(std::any_cast<double>(value));
    }

    template<>
    inline unsigned short ReturnValue::as<unsigned short>() {
        return static_cast<short>(std::any_cast<double>(value));
    }

    template<>
    inline unsigned int ReturnValue::as<unsigned int>() {
        return static_cast<int>(std::any_cast<double>(value));
    }

    template<>
    inline unsigned long ReturnValue::as<unsigned long>() {
        return static_cast<long>(std::any_cast<double>(value));
    }

    template<>
    inline unsigned long long ReturnValue::as<unsigned long long>() {
        return static_cast<unsigned long long>(std::any_cast<double>(value));
    }

    template<>
    inline float ReturnValue::as<float>() {
        return static_cast<float>(std::any_cast<double>(value));
    }

    template<>
    inline double ReturnValue::as<double>() {
        return std::any_cast<double>(value);
    }

    template<>
    inline bool ReturnValue::as<bool>() {
        return std::any_cast<bool>(value);
    }

    template<>
    inline std::string ReturnValue::as<std::string>() {
        return std::any_cast<std::string>(value);
    }

    template<>
    inline std::nullptr_t ReturnValue::as<std::nullptr_t>() {
        return std::any_cast<std::nullptr_t>(value);
    }

    inline void pushArgs(WrenVM* vm, int idx) {
        
    }
    template<typename First, typename... Other>
    inline void pushArgs(WrenVM* vm, int idx, First&& first, Other&&... other) {
        push(vm, idx, first);
        pushArgs(vm, ++idx, std::forward<Other>(other)...);
    }

    template<typename... Args>
    struct CallAndReturn {
        static ReturnValue func(WrenVM* vm, WrenHandle* handle, WrenHandle* func, Args&&... args) {
            constexpr auto n = sizeof...(Args);
            wrenEnsureSlots(vm, n + 1);
            wrenSetSlotHandle(vm, 0, handle);

            pushArgs(vm, 1, std::forward<Args>(args)...);

            auto r = wrenCall(vm, func);
            if (r != WREN_RESULT_SUCCESS) {
                getLastError(vm);
            }

            const auto type = wrenGetSlotType(vm, 0);
            switch(type) {
                case WrenType::WREN_TYPE_BOOL: return ReturnValue(wrenGetSlotBool(vm, 0));
                case WrenType::WREN_TYPE_NUM: return ReturnValue(wrenGetSlotDouble(vm, 0));
                case WrenType::WREN_TYPE_STRING: return ReturnValue(std::string(wrenGetSlotString(vm, 0)));
                case WrenType::WREN_TYPE_FOREIGN: return ReturnValue(wrenGetSlotForeign(vm, 0));
                default: return ReturnValue(nullptr);
            }
        }
    };
} // namespace wrenbind17
