#include <catch2/catch.hpp>
#include <wrenbind17.hpp>

namespace wren = wrenbind17;

template <typename T>
static void sendAndCheck(wren::Method& method, const T& value) {
    //std::cout << "sendAndCheck with T=" << typeid(T).name() << std::endl;
    auto ret = method.operator()(value);
    REQUIRE(ret.template is<T>());
    auto v = ret.template as<T>();
    REQUIRE(v == value);
}

TEST_CASE("Set slot and return by calling Wren"){
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
    
    sendAndCheck<char>(baz, 42);
    sendAndCheck<short>(baz, 42);
    sendAndCheck<int>(baz, 42);
    sendAndCheck<long>(baz, 42);
    sendAndCheck<long long>(baz, 42);
    //sendAndCheck<unsigned char>(baz, 42);
    //sendAndCheck<unsigned short>(baz, 42);
    sendAndCheck<unsigned int>(baz, 42);
    sendAndCheck<unsigned long>(baz, 42);
    sendAndCheck<unsigned long long>(baz, 42);
    sendAndCheck<unsigned>(baz, 42);
    sendAndCheck<float>(baz, 42.0f);
    sendAndCheck<double>(baz, 42.0);
    sendAndCheck<bool>(baz, true);
    sendAndCheck<int8_t>(baz, 42);
    sendAndCheck<int16_t>(baz, 42);
    sendAndCheck<int32_t>(baz, 42);
    sendAndCheck<int64_t>(baz, 42);
    sendAndCheck<uint32_t>(baz, 42);
    sendAndCheck<uint64_t>(baz, 42);
    sendAndCheck<std::string>(baz, std::string("Hello World"));
    sendAndCheck<std::nullptr_t>(baz, nullptr);
}

void* instance = nullptr;

template<typename T>
class GetSlotTest {
public:
    GetSlotTest(T value):value(value){
        instance = this;
    }
    ~GetSlotTest() = default;

    T set(T value) {
        this->value = value;
        return this->value;
    }

    T value;
};

template<typename T>
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

    //std::cout << code << std::endl;

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

TEST_CASE("Get slot by calling C++"){
    testCaseGetSlotByCallingCpp<char>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<short>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<int>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<long>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<long long>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<unsigned int>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<unsigned long>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<unsigned long long>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<bool>("false", false, "true", true);
    testCaseGetSlotByCallingCpp<std::string>("\"Hello\"", "Hello", "\"World\"", "World");
    testCaseGetSlotByCallingCpp<float>("42.1", 42.1, "123.3", 123.3);
    testCaseGetSlotByCallingCpp<double>("42.1", 42.1, "123.3", 123.3);
    testCaseGetSlotByCallingCpp<int8_t>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<int16_t>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<int32_t>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<int64_t>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<uint32_t>("42", 42, "123", 123);
    testCaseGetSlotByCallingCpp<uint64_t>("42", 42, "123", 123);
}

class Vector3 {
public:
    Vector3() = default;
    Vector3(float x, float y, float z):x(x),y(y),z(z){

    }

    void set(float x, float y, float z){
        this->x = x;
        this->y = y;
        this->z = z;
    }

    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

TEST_CASE("Slots as classes"){
    wren::VM vm;

    auto& m = vm.module("test");
    auto& cls = m.klass<Vector3>("Vector3");
    cls.ctor<float, float, float>();
    cls.func<&Vector3::set>("set");

    const std::string code = R"(
        import "test" for Vector3

        class Foo {
            static baz() {
                return Vector3.new(1.1, 2.2, 3.3)
            }
        }
    )";

    vm.runFromSource("main", code);
    auto baz = vm.find("main", "Foo").func("baz()");

    auto ret = baz();
    REQUIRE(ret.is<Vector3>());
    auto v = ret.as<Vector3>();
    REQUIRE(v.x == Approx(1.1f));
    REQUIRE(v.y == Approx(2.2f));
    REQUIRE(v.z == Approx(3.3f));
}

class Foo {
public:
    static void copy(std::string value) {
        str = value;
    }

    static void constCopy(const std::string value) {
        str = value;
    }

    static void constRef(const std::string& value) {
        str = value;
    }

    static std::string str;
};

std::string Foo::str;

TEST_CASE("String slots") {
    const std::string code = R"(
        import "test" for Foo

        class Main {
            static copy(str) {
                return Foo.copy(str)
            }
            static constCopy(str) {
                return Foo.constCopy(str)
            }
            static constRef(str) {
                return Foo.constRef(str)
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    auto& cls = m.klass<Foo>("Foo");
    cls.funcStatic<&Foo::copy>("copy");
    cls.funcStatic<&Foo::constCopy>("constCopy");
    cls.funcStatic<&Foo::constRef>("constRef");

    vm.runFromSource("main", code);
    auto foo = vm.find("main", "Main");
    auto copy = foo.func("copy(_)");
    auto constCopy = foo.func("constCopy(_)");
    auto constRef = foo.func("constRef(_)");

    copy(std::string("Hello World"));
    REQUIRE(Foo::str == "Hello World");
    Foo::str.clear();
    
    constCopy(static_cast<const std::string>(std::string("Hello World")));
    REQUIRE(Foo::str == "Hello World");
    Foo::str.clear();

    auto s = std::string("Hello World");
    const auto& ref = s;
    constRef(s);
    REQUIRE(Foo::str == "Hello World");
    Foo::str.clear();
}
