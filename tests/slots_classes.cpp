#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class Vector3 {
public:
    Vector3() = default;
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {
    }

    void set(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

TEST_CASE("Slots as classes") {
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

    SECTION("String by copy") {
        auto s = std::string("Hello World");
        copy(s);
        REQUIRE(Foo::str == "Hello World");
        Foo::str.clear();
    }

    SECTION("String by move") {
        copy(std::string("Hello World"));
        REQUIRE(Foo::str == "Hello World");
        Foo::str.clear();
    }

    SECTION("String by const copy") {
        constCopy(static_cast<const std::string>(std::string("Hello World")));
        REQUIRE(Foo::str == "Hello World");
        Foo::str.clear();
    }

    SECTION("String by const ref") {
        auto s = std::string("Hello World");
        constRef(s);
        REQUIRE(Foo::str == "Hello World");
        Foo::str.clear();
    }
}
