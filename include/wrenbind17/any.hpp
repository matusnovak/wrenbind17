#pragma once

#include <typeinfo>
#include "handle.hpp"
#include "push.hpp"
#include "pop.hpp"

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

        template <typename T> class Data : public Content {
        public:
            template <typename Arg> Data(Arg arg) : value(std::move(arg)) {
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

        inline Any()
            : content(nullptr) {
        }

        inline explicit Any(WrenVM* vm, const bool value)
            : vm(vm),
              type(WrenType::WREN_TYPE_BOOL),
              content(new Data<bool>(value)) {
        }

        inline explicit Any(WrenVM* vm, const double value)
            : vm(vm),
              type(WrenType::WREN_TYPE_NUM),
              content(new Data<double>(value)) {
        }

        inline explicit Any(WrenVM* vm, std::string value)
            : vm(vm),
              type(WrenType::WREN_TYPE_STRING),
              content(new Data<std::string>(std::move(value))) {
        }

        inline explicit Any(WrenVM* vm, std::nullptr_t value)
            : vm(vm),
              type(WrenType::WREN_TYPE_NULL),
              content(nullptr) {
        }

        inline explicit Any(WrenVM* vm, void* value)
            : vm(vm),
              type(WrenType::WREN_TYPE_FOREIGN),
              content(new Data<void*>(value)) {
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
        inline void swap(Any& other) noexcept {
            std::swap(vm, other.vm);
            std::swap(type, other.type);
            std::swap(content, other.content);
        }

        inline virtual ~Any() = default;

        template <typename T> inline typename std::enable_if<!std::is_pointer<T>::value, T>::type as() const {
            if (empty() || type != WREN_TYPE_FOREIGN)
                throw BadCast("Bad cast when getting value from Wren");
            using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            return *detail::getSlotForeign<Type>(vm, contentCast<void*>().get()).get();
        }

        template <typename T> inline typename std::enable_if<std::is_pointer<T>::value, T>::type as() const {
            if (empty()|| type != WREN_TYPE_FOREIGN)
                throw BadCast("Bad cast when getting value from Wren");
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            return detail::getSlotForeign<Type>(vm, contentCast<void*>().get()).get();
        }

        template <typename T> inline typename std::enable_if<detail::is_shared_ptr<T>::value, T>::type as() const {
            if (empty() || type != WREN_TYPE_FOREIGN)
                throw BadCast("Bad cast when getting value from Wren");
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            return detail::getSlotForeign<Type>(vm, contentCast<void*>().get());
        }

        template <typename T> inline std::shared_ptr<T> shared() const {
            if (empty() || type != WREN_TYPE_FOREIGN)
                throw BadCast("Bad cast when getting value from Wren");
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            return detail::getSlotForeign<Type>(vm, contentCast<void*>().get());
        }

        template <class T> inline bool is() const {
            if (empty())
                return false;
            if (type != WREN_TYPE_FOREIGN)
                return false;
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            const auto foreign = reinterpret_cast<detail::Foreign*>(contentCast<void*>().get());
            return foreign->hash() == typeid(Type).hash_code();
        }

        inline bool empty() const {
            return content == nullptr;
        }

    private:
        template <typename T>
        const Data<T>& contentCast() const {
            if (content == nullptr || content->getTypeid() != typeid(T)) {
                throw BadCast("Bad cast when getting value from Wren");
            }
            return *static_cast<const Data<T>*>(content.get());
        }

        WrenVM* vm = nullptr;
        WrenType type = WrenType::WREN_TYPE_NULL;
        std::unique_ptr<Content> content;
    };

    template <> inline bool Any::is<int8_t>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<char>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<short>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<int>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<long long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<unsigned char>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<unsigned short>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<unsigned int>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<unsigned long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<unsigned long long>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<float>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<double>() const {
        return type == WREN_TYPE_NUM;
    }

    template <> inline bool Any::is<bool>() const {
        return type == WREN_TYPE_BOOL;
    }

    template <> inline bool Any::is<std::nullptr_t>() const {
        return type == WREN_TYPE_NULL;
    }

    template <> inline bool Any::is<std::string>() const {
        return type == WREN_TYPE_STRING;
    }

    template <> inline std::nullptr_t Any::as<std::nullptr_t>() const {
        if (!empty()) {
            throw BadCast("Return value is not null");
        }
        return nullptr;
    }

    template <> inline int8_t Any::as<int8_t>() const {
        return static_cast<int8_t>(contentCast<double>().get());
    }

    template <> inline char Any::as<char>() const {
        return static_cast<char>(contentCast<double>().get());
    }

    template <> inline short Any::as<short>() const {
        return static_cast<short>(contentCast<double>().get());
    }

    template <> inline int Any::as<int>() const {
        return static_cast<int>(contentCast<double>().get());
    }

    template <> inline long Any::as<long>() const {
        return static_cast<long>(contentCast<double>().get());
    }

    template <> inline long long Any::as<long long>() const {
        return static_cast<long long>(contentCast<double>().get());
    }

    template <> inline unsigned char Any::as<unsigned char>() const {
        return static_cast<char>(contentCast<double>().get());
    }

    template <> inline unsigned short Any::as<unsigned short>() const {
        return static_cast<short>(contentCast<double>().get());
    }

    template <> inline unsigned int Any::as<unsigned int>() const {
        return static_cast<int>(contentCast<double>().get());
    }

    template <> inline unsigned long Any::as<unsigned long>() const {
        return static_cast<long>(contentCast<double>().get());
    }

    template <> inline unsigned long long Any::as<unsigned long long>() const {
        return static_cast<unsigned long long>(contentCast<double>().get());
    }

    template <> inline float Any::as<float>() const {
        return static_cast<float>(contentCast<double>().get());
    }

    template <> inline double Any::as<double>() const {
        return contentCast<double>().get();
    }

    template <> inline bool Any::as<bool>() const {
        return contentCast<bool>().get();
    }

    template <> inline std::string Any::as<std::string>() const {
        return contentCast<std::string>().get();
    }

    template <> inline Any detail::getSlot(WrenVM* vm, const int idx) {
        const auto type = wrenGetSlotType(vm, 0);
        switch (type) {
            case WrenType::WREN_TYPE_BOOL:
                return Any(vm, wrenGetSlotBool(vm, 0));
            case WrenType::WREN_TYPE_NUM:
                return Any(vm, wrenGetSlotDouble(vm, 0));
            case WrenType::WREN_TYPE_STRING:
                return Any(vm, std::string(wrenGetSlotString(vm, 0)));
            case WrenType::WREN_TYPE_FOREIGN:
                return Any(vm, wrenGetSlotForeign(vm, 0));
            default:
                return Any(vm, nullptr);
        }
    }
} // namespace wrenbind17
