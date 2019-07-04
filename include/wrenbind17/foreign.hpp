#pragma once

#include <ostream>
#include <unordered_map>
#include <iostream>
#include "allocator.hpp"
#include "caller.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    /**
     * @ingroup wrenbind17
     */
    class ForeignMethod {
    public:
        ForeignMethod(std::string name, WrenForeignMethodFn method, const bool isStatic)
            : name(std::move(name)), method(method), isStatic(isStatic) {
        }
        virtual ~ForeignMethod() = default;
        virtual void generate(std::ostream& os) const = 0;

        const std::string& getName() const {
            return name;
        }

        WrenForeignMethodFn getMethod() const {
            return method;
        }

    protected:
        std::string name;
        WrenForeignMethodFn method;
        bool isStatic;
    };

    /**
     * @ingroup wrenbind17
     */
    class ForeignProp {
    public:
        ForeignProp(std::string name, WrenForeignMethodFn getter, WrenForeignMethodFn setter)
            : name(std::move(name)), getter(getter), setter(setter) {
        }
        virtual ~ForeignProp() = default;

        void generate(std::ostream& os) const {
            if (getter)
                os << "    foreign " << name << "\n";
            if (setter)
                os << "    foreign " << name << "=(rhs)\n";
        }

        const std::string& getName() const {
            return name;
        }

        WrenForeignMethodFn getSetter() {
            return setter;
        }

        WrenForeignMethodFn getGetter() {
            return getter;
        }

    protected:
        std::string name;
        WrenForeignMethodFn getter;
        WrenForeignMethodFn setter;
    };

    /**
     * @ingroup wrenbind17
     */
    class ForeignKlass {
    public:
        ForeignKlass(std::string name) : name(std::move(name)) {
        }
        virtual ~ForeignKlass() = default;
        virtual void generate(std::ostream& os) const = 0;

        ForeignMethod& findFunc(const std::string& name) {
            auto it = methods.find(name);
            if (it == methods.end())
                throw NotFound();
            return *it->second;
        }

        ForeignProp& findProp(const std::string& name) {
            auto it = props.find(name);
            if (it == props.end())
                throw NotFound();
            return *it->second;
        }

        WrenForeignMethodFn findSignature(const std::string& signature) {
            if (signature.find('(') != std::string::npos) {
                // Check if setter
                if (signature.find("=(_)") != std::string::npos) {
                    return findProp(signature.substr(0, signature.find_first_of('='))).getSetter();
                } else {
                    // Must be a method
                    return findFunc(signature.substr(0, signature.find_first_of('('))).getMethod();
                }
            } else {
                return findProp(signature).getGetter();
            }
        }

        const std::string& getName() const {
            return name;
        }

        WrenForeignClassMethods& getAllocators() {
            return allocators;
        }

    protected:
        std::string name;
        std::string ctorDef;
        std::unordered_map<std::string, std::unique_ptr<ForeignMethod>> methods;
        std::unordered_map<std::string, std::unique_ptr<ForeignProp>> props;
        WrenForeignClassMethods allocators;
    };

    /**
     * @ingroup wrenbind17
     */
    template <typename... Args>
    class ForeignMethodImpl : public ForeignMethod {
    public:
        ForeignMethodImpl(std::string name, WrenForeignMethodFn fn, const bool isStatic)
            : ForeignMethod(std::move(name), fn, isStatic) {
        }
        ~ForeignMethodImpl() = default;

        void generate(std::ostream& os) const override {
            constexpr auto n = sizeof...(Args);
            os << "    foreign " << (isStatic ? "static " : "") << name << "(";
            for (size_t i = 0; i < n; i++) {
                if (i == 0)
                    os << "arg0";
                else
                    os << ", arg" << i;
            }
            os << ")\n";
        }
    };

    /**
     * @ingroup wrenbind17
     */
    template <typename T, typename V>
    class ForeignPropImpl : public ForeignProp {
    public:
        ForeignPropImpl(std::string name, WrenForeignMethodFn getter, WrenForeignMethodFn setter)
            : ForeignProp(std::move(name), getter, setter) {
        }
        ~ForeignPropImpl() = default;
    };

    namespace detail {
        template <typename Signature, Signature signature>
        struct ForeignFunctionDetails;

        template <typename R, typename... Args, R (*Fn)(Args...)>
        struct ForeignFunctionDetails<R (*)(Args...), Fn> {
            typedef ForeignMethodImpl<Args...> ForeignMethodImplType;

            static std::unique_ptr<ForeignMethodImplType> make(std::string name) {
                auto p = detail::ForeignFunctionCaller<R, Args...>::template call<Fn>;
                return std::make_unique<ForeignMethodImplType>(std::move(name), p, true);
            }
        };

        template <typename M>
        struct GetPointerType {
            template <typename C, typename T>
            static T getType(T C::*v);

            typedef decltype(getType(static_cast<M>(nullptr))) type;
        };
    } // namespace detail

    /**
     * @ingroup wrenbind17
     */
    template <typename T>
    class ForeignKlassImpl : public ForeignKlass {
    public:
        ForeignKlassImpl(std::string name) : ForeignKlass(std::move(name)) {
        }
        ~ForeignKlassImpl() = default;

        void generate(std::ostream& os) const override {
            os << "foreign class " << name << " {\n";
            if (!ctorDef.empty()) {
                os << "    " << ctorDef;
            }
            for (const auto& pair : methods) {
                pair.second->generate(os);
            }
            for (const auto& pair : props) {
                pair.second->generate(os);
            }
            os << "}\n\n";
        }

        template <typename... Args>
        void ctor(const std::string& name = "new") {
            allocators.allocate = &detail::ForeignKlassAllocator<T, Args...>::allocate;
            allocators.finalize = &detail::ForeignKlassAllocator<T, Args...>::finalize;
            std::stringstream ss;
            ss << "construct " << name << " (";
            constexpr auto n = sizeof...(Args);
            for (size_t i = 0; i < n; i++) {
                if (i == 0)
                    ss << "arg0";
                else
                    ss << ", arg" << i;
            }
            ss << ") {}\n\n";
            ctorDef = ss.str();
        }

        template <typename Signature, Signature signature>
        struct ForeignMethodDetails;

        template <typename R, typename... Args, R (T::*Fn)(Args...)>
        struct ForeignMethodDetails<R (T::*)(Args...), Fn> {
            typedef ForeignMethodImpl<Args...> ForeignMethodImplType;

            static std::unique_ptr<ForeignMethodImplType> make(std::string name) {
                auto p = detail::ForeignMethodCaller<R, T, Args...>::template call<Fn>;
                return std::make_unique<ForeignMethodImplType>(std::move(name), p, false);
            }
        };

        template <typename V, V T::*Ptr>
        struct ForeignVarDetails {
            static std::unique_ptr<ForeignProp> make(std::string name, const bool readonly) {
                auto s = readonly ? nullptr : detail::ForeignPropCaller<T, V, Ptr>::setter;
                auto g = detail::ForeignPropCaller<T, V, Ptr>::getter;
                return std::make_unique<ForeignProp>(std::move(name), g, s);
            }
        };

        template <typename Signature, Signature signature>
        struct ForeignSetterDetails;

        template <typename V, void (T::*Fn)(V)>
        struct ForeignSetterDetails<void (T::*)(V), Fn> {
            static WrenForeignMethodFn method() {
                return detail::ForeignMethodCaller<void, T, V>::template call<Fn>;
            }
        };

        template <typename Signature, Signature signature>
        struct ForeignGetterDetails;

        template <typename R, R (T::*Fn)()>
        struct ForeignGetterDetails<R (T::*)(), Fn> {
            static WrenForeignMethodFn method() {
                return detail::ForeignMethodCaller<R, T>::template call<Fn>;
            }
        };

        template <typename R, R (T::*Fn)() const>
        struct ForeignGetterDetails<R (T::*)() const, Fn> {
            static WrenForeignMethodFn method() {
                return detail::ForeignMethodCaller<R, T>::template call<Fn>;
            }
        };

        template <auto Fn>
        void func(std::string name) {
            auto ptr = ForeignMethodDetails<decltype(Fn), Fn>::make(std::move(name));
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Fn>
        void funcStatic(std::string name) {
            auto ptr = detail::ForeignFunctionDetails<decltype(Fn), Fn>::make(std::move(name));
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Var>
        void var(std::string name) {
            using R = typename detail::GetPointerType<decltype(Var)>::type;
            auto ptr = ForeignVarDetails<R, Var>::make(std::move(name), false);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Var>
        void varReadonly(std::string name) {
            using R = typename detail::GetPointerType<decltype(Var)>::type;
            auto ptr = ForeignVarDetails<R, Var>::make(std::move(name), true);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Getter, auto Setter>
        void prop(std::string name) {
            auto g = ForeignGetterDetails<decltype(Getter), Getter>::method();
            auto s = ForeignSetterDetails<decltype(Setter), Setter>::method();
            auto ptr = std::make_unique<ForeignProp>(std::move(name), g, s);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Getter>
        void propReadonly(std::string name) {
            auto g = ForeignGetterDetails<decltype(Getter), Getter>::method();
            auto ptr = std::make_unique<ForeignProp>(std::move(name), g, nullptr);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }
    };
} // namespace wrenbind17
