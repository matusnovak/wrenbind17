#pragma once

#include "exception.hpp"
#include "handle.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <typeinfo>
#include <wren.hpp>

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    void addClassType(WrenVM* vm, const std::string& module, const std::string& name, size_t hash);
    void getClassType(WrenVM* vm, std::string& module, std::string& name, size_t hash);
    void getLastError(WrenVM* vm);

    inline void exceptionHandler(WrenVM* vm, const std::exception_ptr& eptr) {
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            } else {
                wrenEnsureSlots(vm, 1);
                wrenSetSlotString(vm, 0, "Unknown error");
                wrenAbortFiber(vm, 0);
            }
        } catch (std::exception& e) {
            wrenEnsureSlots(vm, 1);
            wrenSetSlotString(vm, 0, e.what());
            wrenAbortFiber(vm, 0);
        }
    }

    template <class T> struct is_shared_ptr : std::false_type {};
    template <class T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

    namespace detail {
        class Foreign {
        public:
            Foreign() = default;
            virtual ~Foreign() = 0;
            virtual void* get() const = 0;
            virtual size_t hash() const = 0;
        };

        inline Foreign::~Foreign() {
        }

        template <typename T> class ForeignObject : public Foreign {
        public:
            ForeignObject() {
            }
            ForeignObject(std::shared_ptr<T> ptr) : ptr(std::move(ptr)) {
            }
            virtual ~ForeignObject() = default;

            void* get() const override {
                return ptr.get();
            }

            size_t hash() const override {
                return typeid(T).hash_code();
            }

            const std::shared_ptr<T>& shared() const {
                return ptr;
            }

            std::shared_ptr<T> ptr;
        };

        template <class T> struct is_shared_ptr : std::false_type {};
        template <class T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

        template <typename T> inline void setSlot(WrenVM* vm, int idx, T value) {
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
                std::abort();
                throw Exception("Runtime error: Class type not registered in Wren VM (T)");
            }
        }

        template <typename T> struct PushHelper {
            inline static void f(WrenVM* vm, int idx, T value) {
                static_assert(!std::is_pointer<T>::value, "type can't be a pointer");
                setSlot<T>(vm, idx, std::move(value));
            }
        };

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
                    throw Exception("Runtime error: Class type not registered in Wren VM (T*)");
                }
            }
        };

        template <typename T> struct PushHelper<T&> {
            inline static void f(WrenVM* vm, int idx, T& value) {
                setSlot<typename std::remove_const<typename std::remove_reference<T>::type>::type>(vm, idx, value);
                // PushHelper<T*>::f(vm, idx, &value);
            }
        };

        template <typename T> struct PushHelper<const T*> {
            inline static void f(WrenVM* vm, int idx, const T* value) {
                PushHelper<T*>::f(vm, idx, value);
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
                    throw Exception("Runtime error: Class type not registered in Wren VM (shared_ptr<T>)");
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

        /*template <>
        struct PushHelper<const std::string&> {
            inline static void f(WrenVM* vm, int idx, const std::string& value) {
                wrenSetSlotString(vm, idx, value.c_str());
            }
        };*/

        /*template <typename T>
        struct PushHelper<T&> {
            inline static void f(WrenVM* vm, int idx, T& value) {
                setSlot<typename std::remove_reference<T>::type>(vm, idx, value);
            }
        };*/

        /*template <typename T>
        struct SetSlotHelper {
            inline static void f(WrenVM* vm, int idx, T& value) {
                setSlot<T>(vm, idx, value);
            }
        };

        template <typename T>
        struct SetSlotHelper<std::shared_ptr<T>> {
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
                    throw Exception("Runtime error: Class type not registered in Wren VM");
                }
            }
        };

        template <typename T, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = nullptr>
        inline void push(WrenVM* vm, int idx, T value) {
            using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            SetSlotHelper<Type>::f(vm, idx, value);
        }

        template <typename T, typename std::enable_if<std::is_pointer<T>::value, T>::type* = nullptr>
        inline void push(WrenVM* vm, int idx, T value) {
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            try {
                std::string module;
                std::string klass;
                getClassType(vm, module, klass, typeid(Type).hash_code());

                wrenEnsureSlots(vm, idx + 1);
                wrenGetVariable(vm, module.c_str(), klass.c_str(), idx);

                auto memory = wrenSetSlotNewForeign(vm, idx, idx, sizeof(ForeignObject<Type>));
                auto* foreign =
                    new (memory) ForeignObject<Type>(std::shared_ptr<Type>(const_cast<Type*>(value), [](Type* t) {}));
            } catch (std::out_of_range& e) {
                (void)e;
                throw Exception("Runtime error: Class type not registered in Wren VM");
            }
        }*/

        /*template <typename T>
        inline void setSlot(WrenVM* vm, int idx, const T& value) {
            setSlot(vm, idx, const_cast<T&>(value));
        }*/

        template <> inline void setSlot(WrenVM* vm, int idx, std::nullptr_t value) {
            (void)value;
            wrenSetSlotNull(vm, idx);
        }

        template <> inline void setSlot(WrenVM* vm, int idx, std::string value) {
            wrenSetSlotString(vm, idx, value.c_str());
        }

        template <> inline void setSlot(WrenVM* vm, int idx, bool value) {
            wrenSetSlotBool(vm, idx, value);
        }

        template <> inline void setSlot(WrenVM* vm, int idx, int8_t value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, char value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, int value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, short value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, long value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, unsigned long value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, unsigned value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, long long value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, unsigned long long value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, float value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <> inline void setSlot(WrenVM* vm, int idx, double value) {
            wrenSetSlotDouble(vm, idx, static_cast<double>(value));
        }

        template <WrenType Type> inline void validate(WrenVM* vm, int idx) {
            if (wrenGetSlotType(vm, idx) != Type)
                throw BadCast("Bad cast when getting value from Wren");
        }

        template <typename T> T getSlot(WrenVM* vm, int idx) {
            static_assert(!std::is_same<std::string, T>(), "type can't be std::string");
            static_assert(!is_shared_ptr<T>::value, "type can't be shared_ptr<T>");

            validate<WrenType::WREN_TYPE_FOREIGN>(vm, idx);
            using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            auto foreign = reinterpret_cast<ForeignObject<Type>*>(wrenGetSlotForeign(vm, idx));
            if (foreign->hash() != typeid(Type).hash_code()) {
                throw Exception("Invalid foreign object value");
            }
            return *reinterpret_cast<Type*>(foreign->get());
        }

        template <typename T> struct PopHelper {
            static inline T f(WrenVM* vm, int idx) {
                return getSlot<T>(vm, idx);
            }
        };

        template <typename T> struct PopHelper<T*> {
            static inline T* f(WrenVM* vm, int idx) {
                validate<WrenType::WREN_TYPE_FOREIGN>(vm, idx);
                using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
                auto foreign = reinterpret_cast<ForeignObject<Type>*>(wrenGetSlotForeign(vm, idx));
                if (foreign->hash() != typeid(Type).hash_code()) {
                    throw Exception("Invalid foreign object pointer");
                }
                return reinterpret_cast<Type*>(foreign->get());
            }
        };

        template <typename T> struct PopHelper<std::shared_ptr<T>> {
            static inline std::shared_ptr<T> f(WrenVM* vm, int idx) {
                validate<WrenType::WREN_TYPE_FOREIGN>(vm, idx);
                using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
                auto foreign = reinterpret_cast<ForeignObject<Type>*>(wrenGetSlotForeign(vm, idx));
                if (foreign->hash() != typeid(Type).hash_code()) {
                    throw Exception("Invalid foreign object pointer");
                }
                return foreign->shared();
            }
        };

        template <typename T> struct PopHelper<const std::shared_ptr<T>&> {
            static inline std::shared_ptr<T> f(WrenVM* vm, int idx) {
                return PopHelper<std::shared_ptr<T>>::f(vm, idx);
            }
        };

        template <typename T> struct PopHelper<const T&> {
            static inline const T& f(WrenVM* vm, int idx) {
                static_assert(!std::is_same<std::string, T>(), "type can't be std::string");
                static_assert(!is_shared_ptr<T>::value, "type can't be shared_ptr<T>");

                validate<WrenType::WREN_TYPE_FOREIGN>(vm, idx);
                using Type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
                auto foreign = reinterpret_cast<ForeignObject<Type>*>(wrenGetSlotForeign(vm, idx));
                if (foreign->hash() != typeid(Type).hash_code()) {
                    throw Exception("Invalid foreign object pointer");
                }
                return *reinterpret_cast<Type*>(foreign->get());
            }
        };

        /*template <typename T> inline typename std::enable_if<!std::is_pointer<T>::value, T>::type
        pop(WrenVM* vm, int idx) {
            return getSlot<T>(vm, idx);
        }

        template <typename T> inline typename std::enable_if<std::is_pointer<T>::value, T>::type
        pop(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_FOREIGN>(vm, idx);
            using Type = typename std::remove_const<typename std::remove_pointer<T>::type>::type;
            auto foreign = reinterpret_cast<ForeignObject<Type>*>(
                wrenGetSlotForeign(vm, idx)
            );
            if (foreign->hash() != typeid(Type).hash_code()) {
                throw Exception("Invalid foreign object pointer");
            }
            return reinterpret_cast<Type*>(foreign->get());
        }*/

        template <> inline Handle getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_UNKNOWN>(vm, idx);
            return Handle(vm, wrenGetSlotHandle(vm, idx));
        }

        template <> inline std::string getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_STRING>(vm, idx);
            return std::string(wrenGetSlotString(vm, idx));
        }

        template <> struct PopHelper<const std::string&> {
            static inline std::string f(WrenVM* vm, int idx) {
                return getSlot<std::string>(vm, idx);
            }
        };

        template <> inline bool getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_BOOL>(vm, idx);
            return wrenGetSlotBool(vm, idx);
        }

        template <> inline int8_t getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<int8_t>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline char getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<char>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline int getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<int>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline short getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<short>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline long getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<long>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline unsigned long getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<unsigned long>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline unsigned getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<unsigned>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline long long getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<long long>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline unsigned long long getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<unsigned long long>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline float getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<float>(wrenGetSlotDouble(vm, idx));
        }

        template <> inline double getSlot(WrenVM* vm, int idx) {
            validate<WrenType::WREN_TYPE_NUM>(vm, idx);
            return static_cast<double>(wrenGetSlotDouble(vm, idx));
        }
    } // namespace detail
} // namespace wrenbind17
