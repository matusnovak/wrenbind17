#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

template <typename T> static void sendAndCheck(wren::Method& method, const T& value) {
    // std::cout << "sendAndCheck with T=" << typeid(T).name() << std::endl;
    auto ret = method.operator()(value);
    REQUIRE(ret.template is<T>());
    auto v = ret.template as<T>();
    REQUIRE(v == value);
}

TEST_CASE("Set slot and return by calling Wren") {
    wren::VM vm;

    const std::string code = R"(
        class Foo {
            static baz(value) {
                return value
            }
        }
    )";

    vm.runFromSource("main", code);
    auto baz = vm.find("main", "Foo").func("baz(_)");

    SECTION("char") {
        sendAndCheck<char>(baz, 42);
    }
    SECTION("short") {
        sendAndCheck<short>(baz, 42);
    }
    SECTION("int") {
        sendAndCheck<int>(baz, 42);
    }
    SECTION("long") {
        sendAndCheck<long>(baz, 42);
    }
    SECTION("long long") {
        sendAndCheck<long long>(baz, 42);
    }
    SECTION("unsigned char") {
        sendAndCheck<unsigned char>(baz, 42);
    }
    SECTION("unsigned short") {
        sendAndCheck<unsigned short>(baz, 42);
    }
    SECTION("unsigned int") {
        sendAndCheck<unsigned int>(baz, 42);
    }
    SECTION("unsigned long") {
        sendAndCheck<unsigned long>(baz, 42);
    }
    SECTION("unsigned long long>") {
        sendAndCheck<unsigned long long>(baz, 42);
    }
    SECTION("unsigned") {
        sendAndCheck<unsigned>(baz, 42);
    }
    SECTION("float") {
        sendAndCheck<float>(baz, 42.0f);
    }
    SECTION("double") {
        sendAndCheck<double>(baz, 42.0);
    }
    SECTION("bool") {
        sendAndCheck<bool>(baz, true);
    }
    SECTION("int8_t") {
        sendAndCheck<int8_t>(baz, 42);
    }
    SECTION("int16_t") {
        sendAndCheck<int16_t>(baz, 42);
    }
    SECTION("int32_t") {
        sendAndCheck<int32_t>(baz, 42);
    }
    SECTION("int64_t") {
        sendAndCheck<int64_t>(baz, 42);
    }
    SECTION("uint32_t") {
        sendAndCheck<uint32_t>(baz, 42);
    }
    SECTION("uint64_t") {
        sendAndCheck<uint64_t>(baz, 42);
    }
    SECTION("string") {
        sendAndCheck<std::string>(baz, std::string("Hello World"));
    }
    SECTION("nullptr_t") {
        sendAndCheck<std::nullptr_t>(baz, nullptr);
    }
}

void* instance = nullptr;

template <typename T> class GetSlotTest {
public:
    GetSlotTest(T value) : value(value) {
        instance = this;
    }
    ~GetSlotTest() = default;

    T set(T value) {
        this->value = value;
        return this->value;
    }

    T value;
};

template <typename T>
void testCaseGetSlotByCallingCpp(const std::string& initStr, T init, const std::string& nextStr, T next) {
    wren::VM vm;

    typedef GetSlotTest<T> GetSlotTestType;

    auto& m = vm.module("test");
    auto& cls = m.klass<GetSlotTestType>("GetSlotTest");
    cls.template ctor<T>();
    cls.template func<&GetSlotTestType::set>("set");

    std::stringstream ss;
    ss << "import \"test\" for GetSlotTest\n";
    ss << "var Ginstance = GetSlotTest.new(" + initStr + ")\n";
    ss << "class Foo {\n";
    ss << "    static baz() {\n";
    ss << "        return Ginstance.set(" + nextStr + ")\n";
    ss << "    }\n";
    ss << "}\n";
    const auto code = ss.str();

    // std::cout << code << std::endl;

    vm.runFromSource("main", code);
    auto ptr = reinterpret_cast<GetSlotTestType*>(instance);
    REQUIRE(ptr != nullptr);
    REQUIRE(ptr->value == init);

    auto baz = vm.find("main", "Foo").func("baz()");
    auto r = baz(next);
    REQUIRE(r.template is<T>());
    REQUIRE(r.template as<T>() == next);
    REQUIRE(ptr->value == next);
}

TEST_CASE("Get slot by calling C++") {
    SECTION("char") {
        testCaseGetSlotByCallingCpp<char>("42", 42, "123", 123);
    }
    SECTION("short") {
        testCaseGetSlotByCallingCpp<short>("42", 42, "123", 123);
    }
    SECTION("int") {
        testCaseGetSlotByCallingCpp<int>("42", 42, "123", 123);
    }
    SECTION("long") {
        testCaseGetSlotByCallingCpp<long>("42", 42, "123", 123);
    }
    SECTION("long long") {
        testCaseGetSlotByCallingCpp<long long>("42", 42, "123", 123);
    }
    SECTION("unsigned int") {
        testCaseGetSlotByCallingCpp<unsigned int>("42", 42, "123", 123);
    }
    SECTION("unsigned long") {
        testCaseGetSlotByCallingCpp<unsigned long>("42", 42, "123", 123);
    }
    SECTION("unsigned long long") {
        testCaseGetSlotByCallingCpp<unsigned long long>("42", 42, "123", 123);
    }
    SECTION("bool") {
        testCaseGetSlotByCallingCpp<bool>("false", false, "true", true);
    }
    SECTION("string") {
        testCaseGetSlotByCallingCpp<std::string>("\"Hello\"", "Hello", "\"World\"", "World");
    }
    SECTION("float") {
        testCaseGetSlotByCallingCpp<float>("42.1", 42.1f, "123.3", 123.3f);
    }
    SECTION("double") {
        testCaseGetSlotByCallingCpp<double>("42.1", 42.1, "123.3", 123.3);
    }
    SECTION("int8_t") {
        testCaseGetSlotByCallingCpp<int8_t>("42", 42, "123", 123);
    }
    SECTION("int16_t") {
        testCaseGetSlotByCallingCpp<int16_t>("42", 42, "123", 123);
    }
    SECTION("int32_t") {
        testCaseGetSlotByCallingCpp<int32_t>("42", 42, "123", 123);
    }
    SECTION("int64_t") {
        testCaseGetSlotByCallingCpp<int64_t>("42", 42, "123", 123);
    }
    SECTION("uint32_t") {
        testCaseGetSlotByCallingCpp<uint32_t>("42", 42, "123", 123);
    }
    SECTION("uint64_t") {
        testCaseGetSlotByCallingCpp<uint64_t>("42", 42, "123", 123);
    }
}
