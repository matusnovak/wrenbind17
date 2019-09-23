#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

template<typename T>
class PopVariantTestCase {
public:
    using VariantType = typename std::variant<T>;
    class VariantClass;

    class VariantClass {
    public:
        VariantClass() = default;

        void set(VariantType other) {
            value = other;
        }

        VariantType value;
    };

    static void test(const T& value) {
        wren::VM vm;

        const std::string code = R"(
            import "test" for VariantClass
            
            class Main {
                static main(v) {
                    var i = VariantClass.new()
                    i.set(v)
                    return i
                }
            }
        )";

        auto& m = vm.module("test");
        auto& cls = m.klass<VariantClass>("VariantClass");
        cls.ctor();
        cls.template func<&VariantClass::set>("set");

        vm.runFromSource("main", code);

        auto main = vm.find("main", "Main").func("main(_)");
        auto res = main(value);
        REQUIRE(res.template is<VariantClass>());

        auto c = res.template as<VariantClass>();

        REQUIRE(std::holds_alternative<T>(c.value));
        REQUIRE(std::get<T>(c.value) == value);
    }
};

#define TEST_CASE_POP_VARIANT(T, VALUE, NAME)\
    TEST_CASE(NAME) { \
        PopVariantTestCase<T>::test(VALUE); \
    }

TEST_CASE_POP_VARIANT(int, 42, "Call C++ method that accepts a variant of type int");
TEST_CASE_POP_VARIANT(short, 42, "Call C++ method that accepts a variant of type short");
TEST_CASE_POP_VARIANT(long, 42, "Call C++ method that accepts a variant of type long");
TEST_CASE_POP_VARIANT(char, 42, "Call C++ method that accepts a variant of type char");
TEST_CASE_POP_VARIANT(long long, 42, "Call C++ method that accepts a variant of type long long");
TEST_CASE_POP_VARIANT(unsigned int, 42, "Call C++ method that accepts a variant of type unsigned int");
TEST_CASE_POP_VARIANT(unsigned short, 42, "Call C++ method that accepts a variant of type unsigned short");
TEST_CASE_POP_VARIANT(unsigned long, 42, "Call C++ method that accepts a variant of type unsigned long");
TEST_CASE_POP_VARIANT(unsigned char, 42, "Call C++ method that accepts a variant of type unsigned char");
TEST_CASE_POP_VARIANT(unsigned long long, 42, "Call C++ method that accepts a variant of type unsigned long long");
TEST_CASE_POP_VARIANT(bool, true, "Call C++ method that accepts a variant of type bool");
TEST_CASE_POP_VARIANT(float, 42.42f, "Call C++ method that accepts a variant of type float");
TEST_CASE_POP_VARIANT(double, 42.42, "Call C++ method that accepts a variant of type double");
TEST_CASE_POP_VARIANT(std::string, "Hello World", "Call C++ method that accepts a variant of type std::string");
TEST_CASE_POP_VARIANT(std::nullptr_t, nullptr, "Call C++ method that accepts a variant of type std::nullptr_t");

template <typename T> class PopVariantTestCaseConstref {
public:
    using VariantType = typename std::variant<T>;
    class VariantClass;

    class VariantClass {
    public:
        VariantClass() = default;

        void set(const VariantType& other) {
            value = other;
        }

        VariantType value;
    };

    static void test(const T& value) {
        wren::VM vm;

        const std::string code = R"(
            import "test" for VariantClass
            
            class Main {
                static main(v) {
                    var i = VariantClass.new()
                    i.set(v)
                    return i
                }
            }
        )";

        auto& m = vm.module("test");
        auto& cls = m.klass<VariantClass>("VariantClass");
        cls.ctor();
        cls.template func<&VariantClass::set>("set");

        vm.runFromSource("main", code);

        auto main = vm.find("main", "Main").func("main(_)");
        auto res = main(value);
        REQUIRE(res.template is<VariantClass>());

        auto c = res.template as<VariantClass>();

        REQUIRE(std::holds_alternative<T>(c.value));
        REQUIRE(std::get<T>(c.value) == value);
    }
};

#define TEST_CASE_POP_VARIANT_CONSTREF(T, VALUE, NAME)                                                                \
    TEST_CASE(NAME) {                                                                                                  \
        PopVariantTestCaseConstref<T>::test(VALUE);                                                                   \
    }

TEST_CASE_POP_VARIANT_CONSTREF(int, 42, "Call C++ method that accepts a variant const reference of type int");
TEST_CASE_POP_VARIANT_CONSTREF(short, 42, "Call C++ method that accepts a variant const reference of type short");
TEST_CASE_POP_VARIANT_CONSTREF(long, 42, "Call C++ method that accepts a variant const reference of type long");
TEST_CASE_POP_VARIANT_CONSTREF(char, 42, "Call C++ method that accepts a variant const reference of type char");
TEST_CASE_POP_VARIANT_CONSTREF(long long, 42, "Call C++ method that accepts a variant const reference of type long long");
TEST_CASE_POP_VARIANT_CONSTREF(unsigned int, 42,
                                "Call C++ method that accepts a variant const reference of type unsigned int");
TEST_CASE_POP_VARIANT_CONSTREF(unsigned short, 42,
                                "Call C++ method that accepts a variant const reference of type unsigned short");
TEST_CASE_POP_VARIANT_CONSTREF(unsigned long, 42,
                                "Call C++ method that accepts a variant const reference of type unsigned long");
TEST_CASE_POP_VARIANT_CONSTREF(unsigned char, 42,
                                "Call C++ method that accepts a variant const reference of type unsigned char");
TEST_CASE_POP_VARIANT_CONSTREF(unsigned long long, 42,
                                "Call C++ method that accepts a variant const reference of type unsigned long long");
TEST_CASE_POP_VARIANT_CONSTREF(bool, true, "Call C++ method that accepts a variant const reference of type bool");
TEST_CASE_POP_VARIANT_CONSTREF(float, 42.42f, "Call C++ method that accepts a variant const reference of type float");
TEST_CASE_POP_VARIANT_CONSTREF(double, 42.42, "Call C++ method that accepts a variant const reference of type double");
TEST_CASE_POP_VARIANT_CONSTREF(std::string, "Hello World",
                                "Call C++ method that accepts a variant const reference of type std::string");
TEST_CASE_POP_VARIANT_CONSTREF(std::nullptr_t, nullptr,
                                "Call C++ method that accepts a variant const reference of type std::nullptr_t");


template<typename T>
class PushVariantTestCase {
public:
    using VariantType = typename std::variant<T>;
    class VariantClass;

    class VariantClass {
    public:
        VariantClass() = default;

        void set(const T& other) {
            value = other;
        }

        T value;
    };

    static void test(const T& value) {
        wren::VM vm;

        const std::string code = R"(
            import "test" for VariantClass
            
            class Main {
                static main(v) {
                    var i = VariantClass.new()
                    i.set(v)
                    return i
                }
            }
        )";

        auto& m = vm.module("test");
        auto& cls = m.klass<VariantClass>("VariantClass");
        cls.ctor();
        cls.template func<&VariantClass::set>("set");

        vm.runFromSource("main", code);

        auto main = vm.find("main", "Main").func("main(_)");
        auto res = main(VariantType{value});
        REQUIRE(res.template is<VariantClass>());

        auto c = res.template as<VariantClass>();

        REQUIRE(c.value == value);
    }
};

#define TEST_CASE_PUSH_VARIANT(T, VALUE, NAME)\
    TEST_CASE(NAME) { \
        PushVariantTestCase<T>::test(VALUE); \
    }

TEST_CASE_PUSH_VARIANT(int, 42, "Call Wren method that accepts a variant of type int");
TEST_CASE_PUSH_VARIANT(short, 42, "Call Wren method that accepts a variant of type short");
TEST_CASE_PUSH_VARIANT(long, 42, "Call Wren method that accepts a variant of type long");
TEST_CASE_PUSH_VARIANT(char, 42, "Call Wren method that accepts a variant of type char");
TEST_CASE_PUSH_VARIANT(long long, 42, "Call Wren method that accepts a variant of type long long");
TEST_CASE_PUSH_VARIANT(unsigned int, 42, "Call Wren method that accepts a variant of type unsigned int");
TEST_CASE_PUSH_VARIANT(unsigned short, 42, "Call Wren method that accepts a variant of type unsigned short");
TEST_CASE_PUSH_VARIANT(unsigned long, 42, "Call Wren method that accepts a variant of type unsigned long");
TEST_CASE_PUSH_VARIANT(unsigned char, 42, "Call Wren method that accepts a variant of type unsigned char");
TEST_CASE_PUSH_VARIANT(unsigned long long, 42, "Call Wren method that accepts a variant of type unsigned long long");
TEST_CASE_PUSH_VARIANT(bool, true, "Call Wren method that accepts a variant of type bool");
TEST_CASE_PUSH_VARIANT(float, 42.42f, "Call Wren method that accepts a variant of type float");
TEST_CASE_PUSH_VARIANT(double, 42.42, "Call Wren method that accepts a variant of type double");
TEST_CASE_PUSH_VARIANT(std::string, "Hello World", "Call Wren method that accepts a variant of type std::string");
TEST_CASE_PUSH_VARIANT(std::nullptr_t, nullptr, "Call Wren method that accepts a variant of type std::nullptr_t");
