#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class BadMethodClass {
public:
    BadMethodClass() {
    }

    void throwSomething() {
        throw std::runtime_error("Something went wrong");
    }
};

TEST_CASE("Fibre exception bad method") {
    const std::string code = R"(
        import "test" for BadMethodClass

        class Main {
            static main() {
                var fiber = Fiber.new { 
                    var i = BadMethodClass.new()
                    i.throwSomething() 
                }

                var error = fiber.try()
                System.print("Caught error: " + error)
                return error
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<BadMethodClass>("BadMethodClass");
    cls.ctor<>();
    cls.func<&BadMethodClass::throwSomething>("throwSomething");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    auto res = main();
    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == "Something went wrong");
}

class BadClass {
public:
    BadClass() {
        throw std::runtime_error("Something went wrong");
    }
};

TEST_CASE("Fibre exception bad constructor") {
    const std::string code = R"(
        import "test" for BadClass

        class Main {
            static main() {
                var fiber = Fiber.new { 
                    var i = BadClass.new()
                }

                var error = fiber.try()
                System.print("Caught error: " + error)
                return error
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<BadClass>("BadClass");
    cls.ctor<>();

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    auto res = main();
    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == "Something went wrong");
}

TEST_CASE("Exception in constructor") {
    const std::string code = R"(
        import "test" for BadClass

        class Main {
            static main() {
                return BadClass.new()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<BadClass>("BadClass");
    cls.ctor<>();

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    REQUIRE_THROWS_AS(main(), wren::RuntimeError);
}

TEST_CASE("Exception in constructor get error message") {
    const std::string code = R"(
        import "test" for BadClass

        class Main {
            static main() {
                return BadClass.new()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<BadClass>("BadClass");
    cls.ctor<>();

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    try {
        main();
    } catch (wren::RuntimeError& e) {
        std::cout << e.what() << std::endl;
        REQUIRE(std::string(e.what()).find("Runtime error: Something went wrong") == 0);
    }
}

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

class Vector4 {
public:
    Vector4() = default;
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {
    }

    void set(float x, float y, float z, float w) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

class PassToMe {
public:
    PassToMe() {
    }
    virtual ~PassToMe() {
    }

    void set(const Vector3& vector) {
    }
};

TEST_CASE("Passing wrong class must throw exception") {
    const std::string code = R"(
        import "test" for PassToMe, Vector4

        var A = Vector4.new()

        class Main {
            static main() {
                var i = PassToMe.new()
                i.set(A)
                return i
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<Vector4>("Vector4");
    cls.ctor<>();

    auto& cls2 = m.klass<PassToMe>("PassToMe");
    cls2.ctor<>();
    cls2.func<&PassToMe::set>("set");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    REQUIRE_THROWS_AS(main(), wren::RuntimeError);
}

TEST_CASE("Wrong property") {
    const std::string code = R"(
        import "test" for Vector4

        class Main {
            static main() {
                var v = Vector4.new()
                v.a = 1.0
                return v
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<Vector4>("Vector4");
    cls.ctor<>();

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    REQUIRE_THROWS_AS(main(), wren::RuntimeError);
}

TEST_CASE("Compile error") {
    const std::string code = R"(
        import "test" for SomeClassThatDoesNotExist

        class Main {
            static main() {
                var v = SomeClassThatDoesNotExist.new()
                return v
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<Vector4>("Vector4");
    cls.ctor<>();

    REQUIRE_THROWS_AS(vm.runFromSource("main", code), wren::CompileError);
}
