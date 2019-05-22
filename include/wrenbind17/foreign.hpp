#pragma once 

#include <ostream>
#include <unordered_map>
#include <iostream>
#include "allocator.hpp"
#include "caller.hpp"

namespace wrenbind17 {
    class ForeignMethod {
    public:
        ForeignMethod(std::string name, WrenForeignMethodFn method, const bool isStatic) : name(std::move(name)), method(method), isStatic(isStatic) {
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

    class ForeignProp {
    public:
        ForeignProp(std::string name) : name(std::move(name)) {
        }
        virtual ~ForeignProp() = default;
        virtual void generate(std::ostream& os) const = 0;

        const std::string& getName() const {
            return name;
        }

    protected:
        std::string name;
    };

    class ForeignKlass {
    public:
        ForeignKlass(std::string name) : name(std::move(name)) {
        }
        virtual ~ForeignKlass() = default;
        virtual void generate(std::ostream& os) const = 0;

        ForeignMethod& findFunc(const std::string& signature) {
            const auto name = signature.substr(0, signature.find_first_of('('));
            auto it = methods.find(name);
            if (it == methods.end())
                throw NotFound();
            return *it->second;
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

    template <typename... Args>
    class ForeignMethodImpl : public ForeignMethod {
    public:
        ForeignMethodImpl(std::string name, WrenForeignMethodFn fn, const bool isStatic) : ForeignMethod(std::move(name), fn, isStatic) {
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

    template <typename T, typename V>
    class ForeignPropImpl : public ForeignProp {
    public:
        ForeignPropImpl(std::string name, V T::*ptr) : ForeignProp(std::move(name)), ptr(ptr) {
        }
        ~ForeignPropImpl() = default;

        void generate(std::ostream& os) const override {
            os << "    foreign " << name << "\n";
            os << "    foreign " << name << "=(rhs)\n";
        }

    private:
        V T::*ptr;
    };

    template <typename Signature, Signature signature>
    struct ForeignFunctionDetails;

    template <typename R, typename... Args, R (*Fn)(Args...)>
    struct ForeignFunctionDetails<R (*)(Args...), Fn> {
        typedef ForeignMethodImpl<Args...> ForeignMethodImplType;

        static std::unique_ptr<ForeignMethodImplType> make(std::string name) {
            auto p = ForeignFunctionCaller<R, Args...>::template call<Fn>;
            return std::make_unique<ForeignMethodImplType>(std::move(name), p, true);
        }
    };

    template <typename T>
    class ForeignKlassImpl : public ForeignKlass {
    public:
        ForeignKlassImpl(std::string name) : ForeignKlass(std::move(name)) {
        }
        ~ForeignKlassImpl() = default;

        void generate(std::ostream& os) const override {
            os << "foreign class " << name << " {\n";
            if (!ctorDef.empty()) {
                os << "    construct new" << ctorDef << " {}\n\n";
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
        void ctor() {
            allocators.allocate = &ForeignKlassAllocator<T, Args...>::allocate;
            allocators.finalize = &ForeignKlassAllocator<T, Args...>::finalize;
            std::stringstream ss;
            ss << "(";
            constexpr auto n = sizeof...(Args);
            for (size_t i = 0; i < n; i++) {
                if (i == 0)
                    ss << "arg0";
                else
                    ss << ", arg" << i;
            }
            ss << ")";
            ctorDef = ss.str();
        }

        template <typename Signature, Signature signature>
        struct ForeignMethodDetails;

        template <typename R, typename... Args, R (T::*Fn)(Args...)>
        struct ForeignMethodDetails<R (T::*)(Args...), Fn> {
            typedef ForeignMethodImpl<Args...> ForeignMethodImplType;

            static std::unique_ptr<ForeignMethodImplType> make(std::string name) {
                auto p = ForeignMethodCaller<R, T, Args...>::template call<Fn>;
                return std::make_unique<ForeignMethodImplType>(std::move(name), p, false);
            }
        };

        template <auto Fn>
        void func(std::string name) {
            auto ptr = ForeignMethodDetails<decltype(Fn), Fn>::make(std::move(name));
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Fn>
        void funcStatic(std::string name) {
            auto ptr = ForeignFunctionDetails<decltype(Fn), Fn>::make(std::move(name));
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <typename V>
        void prop(std::string name, V T::*ptr) {
            auto p = std::make_unique<ForeignPropImpl<T, V>>(std::move(name), ptr);
            props.insert(std::make_pair(p->getName(), std::move(p)));
        }
    };
}
