#include <catch2/catch.hpp>
#include <wrenbind17.hpp>

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

TEST_CASE("Pass class from Wren to C++") {
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
        REQUIRE(instances.size() >= 2);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Const copy") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstCopy>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
        REQUIRE(instances.size() >= 2);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Reference") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromRef>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
        REQUIRE(instances.size() == 1);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Const reference") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
        REQUIRE(instances.size() == 1);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Pointer") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromPtr>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
        REQUIRE(instances.size() == 1);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Const pointer") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstPtr>("set");
        vm.runFromSource("main", code);
        auto baz = vm.find("main", "Foo").func("baz()");
        baz();
        REQUIRE(instances.size() == 1);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();
}

TEST_CASE("Pass class from C++ to Wren") {
    const std::string code = R"(
        import "test" for Vector3
        var A = Vector3.new(0.0, 0.0, 0.0)
        class Foo {
            static baz(other) {
                A.set(other)
            }
        }
    )";

    SECTION("Copy") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");

        vm.runFromSource("main", code);
        Vector3 value(1.1, 2.2, 3.3);
        auto baz = vm.find("main", "Foo").func("baz(_)");
        baz(value);

        // REQUIRE(instances.size() == 4);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Const copy") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");

        vm.runFromSource("main", code);
        const Vector3 value(1.1, 2.2, 3.3);
        auto baz = vm.find("main", "Foo").func("baz(_)");
        baz(value);

        // REQUIRE(instances.size() == 4);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Reference") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");

        vm.runFromSource("main", code);
        Vector3 value(1.1, 2.2, 3.3);
        auto& ref = value;
        auto baz = vm.find("main", "Foo").func("baz(_)");
        baz(ref);

        // REQUIRE(instances.size() == 4);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Const reference") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");

        vm.runFromSource("main", code);
        Vector3 value(1.1, 2.2, 3.3);
        const auto& ref = value;
        auto baz = vm.find("main", "Foo").func("baz(_)");
        baz(ref);

        // REQUIRE(instances.size() == 4);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Pointer") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");

        vm.runFromSource("main", code);
        Vector3 value(1.1, 2.2, 3.3);
        auto baz = vm.find("main", "Foo").func("baz(_)");
        baz(&value);

        // REQUIRE(instances.size() == 4);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();

    SECTION("Const pointer") {
        TEST_SETUP_VECTOR
        cls.func<&Vector3::setFromConstRef>("set");

        vm.runFromSource("main", code);
        Vector3 value(1.1, 2.2, 3.3);
        const auto* ptr = &value;
        auto baz = vm.find("main", "Foo").func("baz(_)");
        baz(ptr);

        // REQUIRE(instances.size() == 4);
        REQUIRE(instances[0]->x == Approx(1.1));
        REQUIRE(instances[0]->y == Approx(2.2));
        REQUIRE(instances[0]->z == Approx(3.3));
    }
    instances.clear();
}

class Widget;
std::vector<Widget*> widgets;

class Widget {
public:
    Widget() = delete;
    Widget(std::string name) : name(std::move(name)) {
        widgets.push_back(this);
    }
    Widget(Widget&& other) = delete;
    Widget(const Widget& other) = delete;
    ~Widget() = default;

    void set(const Widget& other) {
        name = other.name;
    }

    Widget& operator=(const Widget& other) = delete;
    Widget& operator=(Widget&& other) = delete;

    std::string name;
};

TEST_CASE("Pass class from C++ to Wren as shared_ptr") {
    const std::string code = R"(
        import "test" for Widget
        var A = Widget.new("Ahoj")
        class Foo {
            static baz(other) {
                A.set(other)
            }
        }
    )";

    SECTION("std::shared_ptr") {
        wren::VM vm;
        auto& m = vm.module("test");
        auto& cls = m.klass<Widget>("Widget");
        cls.ctor<std::string>();
        cls.func<&Widget::set>("set");

        vm.runFromSource("main", code);
        auto ptr = std::make_shared<Widget>("Hello World");
        auto baz = vm.find("main", "Foo").func("baz(_)");
        baz(ptr);

        REQUIRE(widgets.size() == 2);
        REQUIRE(widgets[0]->name == "Hello World");
    }
    instances.clear();
}

class Callbacks {
public:
    static void add(wren::Variable instance, std::string funcName) {
        // We have to remove last character because )" is not valid
        // inside of std::string ... = R"()"
        // So I have added a single space character.
        callback = instance.func(funcName.substr(0, funcName.size() - 1));
    }

    static wren::Method callback;
};

wren::Method Callbacks::callback;

TEST_CASE("Callbacks") {
    const std::string code = R"(
        import "test" for Callbacks
        class Foo {
            msg { _msg }
            msg=(msg) { _msg = msg }

            construct new(msg) {
                this.msg = msg
            }

            test() {
                //System.print("Sending back %(this.msg) ")
                return this.msg
            }
        }
        var Instance = Foo.new("Hello World")
        class Main {
            static main() {
                Callbacks.add(Instance, "test() ")
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<Callbacks>("Callbacks");
    cls.funcStatic<&Callbacks::add>("add");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    main();

    auto r = Callbacks::callback();
    REQUIRE(r.is<std::string>());
    REQUIRE(r.as<std::string>() == "Hello World");
    Callbacks::callback.reset();
}
