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
    enum ForeignMethodOperator {
        OPERATOR_GET_INDEX,
        OPERATOR_SET_INDEX,
        OPERATOR_SUB,
        OPERATOR_ADD,
        OPERATOR_MUL,
        OPERATOR_DIV,
        OPERATOR_NEG,
        OPERATOR_MOD,
        OPERATOR_EQUAL,
        OPERATOR_NOT_EQUAL,
        OPERATOR_GT,
        OPERATOR_LT,
        OPERATOR_GT_EQUAL,
        OPERATOR_LT_EQUAL,
        OPERATOR_SHIFT_LEFT,
        OPERATOR_SHIFT_RIGHT,
        OPERATOR_AND,
        OPERATOR_XOR,
        OPERATOR_OR
    };
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

        bool getStatic() const {
            return isStatic;
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
        ForeignProp(std::string name, WrenForeignMethodFn getter, WrenForeignMethodFn setter, const bool isStatic)
            : name(std::move(name)), getter(getter), setter(setter), isStatic(isStatic) {
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

        bool getStatic() const {
            return isStatic;
        }
    protected:
        std::string name;
        WrenForeignMethodFn getter;
        WrenForeignMethodFn setter;
        bool isStatic;
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

        ForeignMethod& findFunc(const std::string& name, const bool isStatic) {
            const auto it = methods.find(name);
            if (it == methods.end())
                throw NotFound();
            if (it->second->getStatic() != isStatic)
                throw NotFound();
            return *it->second;
        }

        ForeignProp& findProp(const std::string& name, const bool isStatic) {
            const auto it = props.find(name);
            if (it == props.end())
                throw NotFound();
            if (it->second->getStatic() != isStatic)
                throw NotFound();
            return *it->second;
        }

        WrenForeignMethodFn findSignature(const std::string& signature, const bool isStatic) {
            switch (signature[0]) {
                case '[':
                case '-':
                case '+':
                case '/':
                case '*':
                case '=':
                case '!':
                case '%':
                case '<':
                case '>':
                case '&':
                case '^':
                case '|': {
                    // Operators
                    return findFunc(signature, isStatic).getMethod();
                }
                default: {
                    if (signature.find('(') != std::string::npos) {
                        // Check if setter
                        if (signature.find("=(_)") != std::string::npos) {
                            return findProp(signature.substr(0, signature.find_first_of('=')), isStatic).getSetter();
                        } else {
                            // Must be a method
                            return findFunc(signature, isStatic).getMethod();
                        }

                    } else {
                        return findProp(signature, isStatic).getGetter();
                    }
                }
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
        ForeignMethodImpl(std::string name, std::string signature, WrenForeignMethodFn fn, const bool isStatic)
            : ForeignMethod(std::move(name), fn, isStatic), signature(std::move(signature)) {
        }
        ~ForeignMethodImpl() = default;

        void generate(std::ostream& os) const override {
            constexpr auto n = sizeof...(Args);
            os << "    foreign " << (isStatic ? "static " : "") << signature << "\n";
        }

        static std::string generateSignature(const std::string& name) {
            std::stringstream os;
            constexpr auto n = sizeof...(Args);
            os << name << "(";
            for (size_t i = 0; i < n; i++) {
                if (i == 0)
                    os << "arg0";
                else
                    os << ", arg" << i;
            }
            os << ")";
            return os.str();
        }

        static std::string generateSignature(const ForeignMethodOperator name) {
            switch (name) {
                case OPERATOR_GET_INDEX:
                    return "[arg]";
                case OPERATOR_SET_INDEX:
                    return "[arg]=(rhs)";
                case OPERATOR_ADD:
                    return "+(rhs)";
                case OPERATOR_SUB:
                    return "-(rhs)";
                case OPERATOR_DIV:
                    return "/(rhs)";
                case OPERATOR_MUL:
                    return "*(rhs)";
                case OPERATOR_MOD:
                    return "%(rhs)";
                case OPERATOR_EQUAL:
                    return "==(rhs)";
                case OPERATOR_NOT_EQUAL:
                    return "!=(rhs)";
                case OPERATOR_NEG:
                    return "-";
                case OPERATOR_GT:
                    return ">(rhs)";
                case OPERATOR_LT:
                    return "<(rhs)";
                case OPERATOR_GT_EQUAL:
                    return ">=(rhs)";
                case OPERATOR_LT_EQUAL:
                    return "<=(rhs)";
                case OPERATOR_SHIFT_LEFT:
                    return "<<(rhs)";
                case OPERATOR_SHIFT_RIGHT:
                    return ">>(rhs)";
                case OPERATOR_AND:
                    return "&(rhs)";
                case OPERATOR_XOR:
                    return "^(rhs)";
                case OPERATOR_OR:
                    return "|(rhs)";
                default:
                    throw Exception("Operator not supported");
            }
        }

        static std::string generateName(const ForeignMethodOperator name) {
            switch (name) {
                case OPERATOR_GET_INDEX:
                    return "[_]";
                case OPERATOR_SET_INDEX:
                    return "[_]=(_)";
                case OPERATOR_ADD:
                    return "+(_)";
                case OPERATOR_SUB:
                    return "-(_)";
                case OPERATOR_DIV:
                    return "/(_)";
                case OPERATOR_MUL:
                    return "*(_)";
                case OPERATOR_MOD:
                    return "%(_)";
                case OPERATOR_EQUAL:
                    return "==(_)";
                case OPERATOR_NOT_EQUAL:
                    return "!=(_)";
                case OPERATOR_NEG:
                    return "-";
                case OPERATOR_GT:
                    return ">(_)";
                case OPERATOR_LT:
                    return "<(_)";
                case OPERATOR_GT_EQUAL:
                    return ">=(_)";
                case OPERATOR_LT_EQUAL:
                    return "<=(_)";
                case OPERATOR_SHIFT_LEFT:
                    return "<<(_)";
                case OPERATOR_SHIFT_RIGHT:
                    return ">>(_)";
                case OPERATOR_AND:
                    return "&(_)";
                case OPERATOR_XOR:
                    return "^(_)";
                case OPERATOR_OR:
                    return "|(_)";
                default:
                    throw Exception("Operator not supported");
            }
        }
    private:
        std::string signature;
    };

    /**
     * @ingroup wrenbind17
     */
    template <typename T, typename V>
    class ForeignPropImpl : public ForeignProp {
    public:
        ForeignPropImpl(std::string name, WrenForeignMethodFn getter, WrenForeignMethodFn setter, const bool isStatic)
            : ForeignProp(std::move(name), getter, setter, isStatic) {
        }
        ~ForeignPropImpl() = default;
    };

    namespace detail {
        template <typename... Args>
        inline std::string generateNameArgs() {
            constexpr auto n = sizeof...(Args);
            std::stringstream ss;
            ss << "(";
            for (size_t i = 0; i < n; i++) {
                ss << "_";
                if (i != n - 1) {
                    ss << ",";
                }
            }
            ss << ")";
            return ss.str();
        }

        template <typename Signature, Signature signature>
        struct ForeignFunctionDetails;

        template <typename R, typename... Args, R (*Fn)(Args...)>
        struct ForeignFunctionDetails<R (*)(Args...), Fn> {
            typedef ForeignMethodImpl<Args...> ForeignMethodImplType;

            static std::unique_ptr<ForeignMethodImplType> make(std::string name) {
                auto signature = ForeignMethodImplType::generateSignature(name);
                auto p = detail::ForeignFunctionCaller<R, Args...>::template call<Fn>;
                name = name + detail::generateNameArgs<Args...>();
                return std::make_unique<ForeignMethodImplType>(std::move(name), std::move(signature), p, true);
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
            allocators.allocate = nullptr;
            allocators.finalize = nullptr;
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
                auto signature = ForeignMethodImplType::generateSignature(name);
                auto p = detail::ForeignMethodCaller<R, T, Args...>::template call<Fn>;
                name = name + detail::generateNameArgs<Args...>();
                return std::make_unique<ForeignMethodImplType>(std::move(name), std::move(signature), p, false);
            }

            static std::unique_ptr<ForeignMethodImplType> make(const ForeignMethodOperator op) {
                auto signature = ForeignMethodImplType::generateSignature(op);
                auto name = ForeignMethodImplType::generateName(op);
                auto p = detail::ForeignMethodCaller<R, T, Args...>::template call<Fn>;
                return std::make_unique<ForeignMethodImplType>(std::move(name), std::move(signature), p, false);
            }
        };

        template <typename R, typename... Args, R (T::*Fn)(Args...) const>
        struct ForeignMethodDetails<R (T::*)(Args...) const, Fn> {
            typedef ForeignMethodImpl<Args...> ForeignMethodImplType;

            static std::unique_ptr<ForeignMethodImplType> make(std::string name) {
                auto signature = ForeignMethodImplType::generateSignature(name);
                auto p = detail::ForeignMethodCaller<R, T, Args...>::template call<Fn>;
                name = name + detail::generateNameArgs<Args...>();
                return std::make_unique<ForeignMethodImplType>(std::move(name), std::move(signature), p, false);
            }

            static std::unique_ptr<ForeignMethodImplType> make(const ForeignMethodOperator op) {
                auto signature = ForeignMethodImplType::generateSignature(op);
                auto name = ForeignMethodImplType::generateName(op);
                auto p = detail::ForeignMethodCaller<R, T, Args...>::template call<Fn>;
                return std::make_unique<ForeignMethodImplType>(std::move(name), std::move(signature), p, false);
            }
        };

        template <typename Signature, Signature signature>
        struct ForeignMethodExtDetails;

        template <typename R, typename... Args, R (*Fn)(T&, Args...)>
        struct ForeignMethodExtDetails<R (*)(T&, Args...), Fn> {
            typedef ForeignMethodImpl<Args...> ForeignMethodImplType;

            static std::unique_ptr<ForeignMethodImplType> make(std::string name) {
                auto signature = ForeignMethodImplType::generateSignature(name);
                auto p = detail::ForeignMethodExtCaller<R, T, Args...>::template call<Fn>;
                name = name + detail::generateNameArgs<Args...>();
                return std::make_unique<ForeignMethodImplType>(std::move(name), std::move(signature), p, false);
            }

            static std::unique_ptr<ForeignMethodImplType> make(const ForeignMethodOperator op) {
                auto signature = ForeignMethodImplType::generateSignature(op);
                auto name = ForeignMethodImplType::generateName(op);
                auto p = detail::ForeignMethodExtCaller<R, T, Args...>::template call<Fn>;
                return std::make_unique<ForeignMethodImplType>(std::move(name), std::move(signature), p, false);
            }
        };

        template <typename V, V T::*Ptr>
        struct ForeignVarDetails {
            static std::unique_ptr<ForeignProp> make(std::string name, const bool readonly) {
                auto s = readonly ? nullptr : detail::ForeignPropCaller<T, V, Ptr>::setter;
                auto g = detail::ForeignPropCaller<T, V, Ptr>::getter;
                return std::make_unique<ForeignProp>(std::move(name), g, s, false);
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
        struct ForeignSetterExtDetails;

        template <typename V, void (*Fn)(T&, V)>
        struct ForeignSetterExtDetails<void (*)(T&, V), Fn> {
            static WrenForeignMethodFn method() {
                return detail::ForeignMethodExtCaller<void, T, V>::template call<Fn>;
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

        template <typename Signature, Signature signature>
        struct ForeignGetterExtDetails;

        template <typename R, R (*Fn)(T&)>
        struct ForeignGetterExtDetails<R (*)(T&), Fn> {
            static WrenForeignMethodFn method() {
                return detail::ForeignMethodExtCaller<R, T>::template call<Fn>;
            }
        };

        template <auto Fn>
        void func(std::string name) {
            auto ptr = ForeignMethodDetails<decltype(Fn), Fn>::make(std::move(name));
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Fn>
        void func(const ForeignMethodOperator name) {
            auto ptr = ForeignMethodDetails<decltype(Fn), Fn>::make(name);
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Fn>
        void funcExt(std::string name) {
            auto ptr = ForeignMethodExtDetails<decltype(Fn), Fn>::make(std::move(name));
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Fn>
        void funcExt(const ForeignMethodOperator name) {
            auto ptr = ForeignMethodExtDetails<decltype(Fn), Fn>::make(name);
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Fn>
        void funcStatic(std::string name) {
            auto ptr = detail::ForeignFunctionDetails<decltype(Fn), Fn>::make(std::move(name));
            methods.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Fn>
        void funcStaticExt(std::string name) {
            // This is exactly the same as funcStatic because there is 
            // no difference for "static void Foo::foo(){}" and "void foo(){}"!
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
            auto ptr = std::make_unique<ForeignProp>(std::move(name), g, s, false);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Getter>
        void propReadonly(std::string name) {
            auto g = ForeignGetterDetails<decltype(Getter), Getter>::method();
            auto ptr = std::make_unique<ForeignProp>(std::move(name), g, nullptr, false);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Getter, auto Setter>
        void propExt(std::string name) {
            auto g = ForeignGetterExtDetails<decltype(Getter), Getter>::method();
            auto s = ForeignSetterExtDetails<decltype(Setter), Setter>::method();
            auto ptr = std::make_unique<ForeignProp>(std::move(name), g, s, false);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }

        template <auto Getter>
        void propReadonlyExt(std::string name) {
            auto g = ForeignGetterExtDetails<decltype(Getter), Getter>::method();
            auto ptr = std::make_unique<ForeignProp>(std::move(name), g, nullptr, false);
            props.insert(std::make_pair(ptr->getName(), std::move(ptr)));
        }
    };
} // namespace wrenbind17
