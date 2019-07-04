#pragma once

#include <typeinfo>
#include "handle.hpp"
#include "slots.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    /**
     * @ingroup wrenbind17
     */
    class Any {
    public:
        class Content {
        public:
            virtual ~Content() {
            }
            virtual const std::type_info& getTypeid() const = 0;
        };

        template <typename T>
        class Data : public Content {
        public:
            template <class... Args>
            Data(Args&&... args) : value(std::forward<Args>(args)...) {
            }
            virtual ~Data() = default;
            const std::type_info& getTypeid() const override {
                return typeid(T);
            }
            T& get() {
                return value;
            }
            const T& get() const {
                return value;
            }

        private:
            T value;
        };

        inline Any() : content(nullptr) {
        }

        inline Any(const Any& other) = delete;
        inline Any(Any&& other) noexcept {
            swap(other);
        }
        inline Any& operator=(const Any& other) = delete;
        inline Any& operator=(Any&& other) noexcept {
            if (this != &other) {
                swap(other);
            }
            return *this;
        }
        inline void swap(Any& other) {
            std::swap(content, other.content);
        }

        template <typename T>
        inline Any(T value) : content(new Data<T>{value}) {
        }

        inline virtual ~Any() = default;

        template <typename T>
        inline T as() const {
            if (content == nullptr || content->getTypeid() != typeid(T)) {
                throw BadCast("Invalid cast on return value");
            }
            return static_cast<const Data<T>&>(*content.get()).get();
        }

        template <class T>
        inline bool is() const {
            if (empty())
                return false;
            return (content->getTypeid() == typeid(T));
        }

        inline bool empty() const {
            return content == nullptr;
        }

    private:
        std::unique_ptr<Content> content;
    };

    template <>
    inline std::nullptr_t Any::as() const {
        if (!empty()) {
            throw BadCast("Return value is not null");
        }
        return nullptr;
    }

    /**
     * @ingroup wrenbind17
     */
    class ReturnValue {
    public:
        explicit ReturnValue(double value) : type{WrenType::WREN_TYPE_NUM}, value{value} {};

        explicit ReturnValue(bool value) : type{WrenType::WREN_TYPE_BOOL}, value{value} {};

        explicit ReturnValue(std::nullptr_t value) : type{WrenType::WREN_TYPE_NULL}, value{} {
            (void)value;
        };

        explicit ReturnValue(std::string value) : type{WrenType::WREN_TYPE_STRING}, value{std::move(value)} {};

        explicit ReturnValue(void* value) : type{WrenType::WREN_TYPE_FOREIGN}, value{std::move(value)} {};

        template <typename T, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = nullptr>
        bool is() const {
            if (type != WREN_TYPE_FOREIGN)
                return false;
            using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            auto foreign = reinterpret_cast<detail::ForeignObject<Type>*>(value.as<void*>());
            return foreign->hash() == typeid(Type).hash_code();
        }

        template <typename T, typename std::enable_if<std::is_pointer<T>::value, T>::type* = nullptr>
        bool is() const {
            if (type != WREN_TYPE_FOREIGN)
                return false;
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            auto foreign = reinterpret_cast<detail::ForeignObject<Type>*>(value.as<void*>());
            return foreign->hash() == typeid(Type).hash_code();
        }

        template <typename T>
        inline typename std::enable_if<!std::is_pointer<T>::value, T>::type as() {
            if (type != WREN_TYPE_FOREIGN)
                throw BadCast("Return value is not foreign");
            using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            auto foreign = reinterpret_cast<detail::ForeignObject<Type>*>(value.as<void*>());
            if (foreign->hash() != typeid(Type).hash_code()) {
                throw BadCast("Return value does not match the expected type");
            }
            return *foreign->shared().get();
        }

        template <typename T>
        inline typename std::enable_if<std::is_pointer<T>::value, T>::type as() {
            if (type == WREN_TYPE_NULL)
                return nullptr;
            if (type != WREN_TYPE_FOREIGN)
                throw BadCast("Return value is not foreign");
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            auto foreign = reinterpret_cast<detail::ForeignObject<Type>*>(value.as<void*>());
            if (foreign->hash() != typeid(Type).hash_code()) {
                throw BadCast("Return value does not match the expected type");
            }
            return foreign->shared().get();
        }

        template <typename T>
        inline typename std::shared_ptr<T> shared() {
            if (type == WREN_TYPE_NULL)
                return nullptr;
            if (type != WREN_TYPE_FOREIGN)
                throw BadCast("Return value is not foreign");
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            auto foreign = reinterpret_cast<detail::ForeignObject<Type>*>(value.as<void*>());
            if (foreign->hash() != typeid(Type).hash_code()) {
                throw BadCast("Return value does not match the expected type");
            }
            return foreign->shared();
        }

    private:
        WrenType type;
        Any value;
    };

    template <>
    inline bool ReturnValue::is<int8_t>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<char>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<short>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<int>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<long long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<unsigned char>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<unsigned short>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<unsigned int>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<unsigned long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<unsigned long long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<float>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<double>() const {
        return type == WREN_TYPE_NUM;
    }

    template <>
    inline bool ReturnValue::is<bool>() const {
        return type == WREN_TYPE_BOOL;
    }

    template <>
    inline bool ReturnValue::is<std::nullptr_t>() const {
        return type == WREN_TYPE_NULL;
    }

    template <>
    inline bool ReturnValue::is<std::string>() const {
        return type == WREN_TYPE_STRING;
    }

    template <>
    inline int8_t ReturnValue::as<int8_t>() {
        return static_cast<int8_t>(value.as<double>());
    }

    template <>
    inline char ReturnValue::as<char>() {
        return static_cast<char>(value.as<double>());
    }

    template <>
    inline short ReturnValue::as<short>() {
        return static_cast<short>(value.as<double>());
    }

    template <>
    inline int ReturnValue::as<int>() {
        return static_cast<int>(value.as<double>());
    }

    template <>
    inline long ReturnValue::as<long>() {
        return static_cast<long>(value.as<double>());
    }

    template <>
    inline long long ReturnValue::as<long long>() {
        return static_cast<long long>(value.as<double>());
    }

    template <>
    inline unsigned char ReturnValue::as<unsigned char>() {
        return static_cast<char>(value.as<double>());
    }

    template <>
    inline unsigned short ReturnValue::as<unsigned short>() {
        return static_cast<short>(value.as<double>());
    }

    template <>
    inline unsigned int ReturnValue::as<unsigned int>() {
        return static_cast<int>(value.as<double>());
    }

    template <>
    inline unsigned long ReturnValue::as<unsigned long>() {
        return static_cast<long>(value.as<double>());
    }

    template <>
    inline unsigned long long ReturnValue::as<unsigned long long>() {
        return static_cast<unsigned long long>(value.as<double>());
    }

    template <>
    inline float ReturnValue::as<float>() {
        return static_cast<float>(value.as<double>());
    }

    template <>
    inline double ReturnValue::as<double>() {
        return value.as<double>();
    }

    template <>
    inline bool ReturnValue::as<bool>() {
        return value.as<bool>();
    }

    template <>
    inline std::string ReturnValue::as<std::string>() {
        return value.as<std::string>();
    }

    template <>
    inline std::nullptr_t ReturnValue::as<std::nullptr_t>() {
        return value.as<std::nullptr_t>();
    }

    namespace detail {
        inline void pushArgs(WrenVM* vm, int idx) {
            (void)vm;
            (void)idx;
        }
        template <typename First, typename... Other>
        inline void pushArgs(WrenVM* vm, int idx, First&& first, Other&&... other) {
            PushHelper<First>::f(vm, idx, first);
            pushArgs(vm, ++idx, std::forward<Other>(other)...);
        }

        template <typename... Args>
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
                switch (type) {
                    case WrenType::WREN_TYPE_BOOL:
                        return ReturnValue(wrenGetSlotBool(vm, 0));
                    case WrenType::WREN_TYPE_NUM:
                        return ReturnValue(wrenGetSlotDouble(vm, 0));
                    case WrenType::WREN_TYPE_STRING:
                        return ReturnValue(std::string(wrenGetSlotString(vm, 0)));
                    case WrenType::WREN_TYPE_FOREIGN:
                        return ReturnValue(wrenGetSlotForeign(vm, 0));
                    default:
                        return ReturnValue(nullptr);
                }
            }
        };
    } // namespace detail
} // namespace wrenbind17
