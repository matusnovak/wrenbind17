#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class Vector3;
std::vector<Vector3*> instances;

class Vector3 {
public:
    Vector3() {
        instances.push_back(this);
    }
    Vector3(double x, double y, double z) : x(x), y(y), z(z) {
        instances.push_back(this);
    }
    Vector3(const Vector3& other) {
        instances.push_back(this);
        *this = other;
    }
    ~Vector3() = default;

    void set(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    void setFromCopy(Vector3 other) {
        *this = other;
    }

    void setFromConstCopy(const Vector3 other) {
        *this = other;
    }

    void setFromRef(Vector3& other) {
        *this = other;
    }

    void setFromConstRef(const Vector3& other) {
        *this = other;
    }

    void setFromPtr(Vector3* other) {
        *this = *other;
    }

    void setFromConstPtr(const Vector3* other) {
        *this = *other;
    }

    Vector3& operator=(const Vector3& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    double x = 0.0f;
    double y = 0.0f;
    double z = 0.0f;
};

#define TEST_SETUP_VECTOR                                                                                              \
    wren::VM vm;                                                                                                       \
    auto& m = vm.module("test");                                                                                       \
    auto& cls = m.klass<Vector3>("Vector3");                                                                           \
    cls.ctor<double, double, double>();

class Vector3Any {
public:
    Vector3Any(double x, double y, double z) : x(x), y(y), z(z) {
    }
    ~Vector3Any() = default;

    void setFromCopy(wren::Any other) {
        REQUIRE(other.is<Vector3Any>());
        *this = other.as<Vector3Any>();
        REQUIRE(x == Approx(1.1));
    }

    void setFromConstCopy(const wren::Any other) {
        REQUIRE(other.is<Vector3Any>());
        *this = other.as<Vector3Any>();
        REQUIRE(x == Approx(1.1));
    }

    void setFromRef(wren::Any& other) {
        REQUIRE(other.is<Vector3Any>());
        *this = other.as<Vector3Any>();
        REQUIRE(x == Approx(1.1));
    }

    void setFromConstRef(const wren::Any& other) {
        REQUIRE(other.is<Vector3Any>());
        *this = other.as<Vector3Any>();
        REQUIRE(x == Approx(1.1));
    }

    Vector3Any& operator=(const Vector3Any& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    double x = 0.0f;
    double y = 0.0f;
    double z = 0.0f;
};

#define TEST_SETUP_VECTOR_ANY                                                                                          \
    wren::VM vm;                                                                                                       \
    auto& m = vm.module("test");                                                                                       \
    auto& cls = m.klass<Vector3Any>("Vector3");                                                                        \
    cls.ctor<double, double, double>();

TEST_CASE("Pass class from Wren to C++ via Any") {
    const std::string code = R"(
        import "test" for Vector3
        var A = Vector3.new(1.1, 2.2, 3.3)
        class Foo {
            static baz() {
                A.set(A)
            }
        }
    )";

    SECTION("Copy") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromCopy>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
    }

    SECTION("Const copy") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstCopy>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
    }

    SECTION("Reference") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromRef>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
    }

    SECTION("Const reference") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
    }
}
