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

    void from(const Vector4& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
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
        import "test" for ExceptionClassThatDoesNotExist

        class Main {
            static main() {
                var v = ExceptionClassThatDoesNotExist.new()
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

TEST_CASE("Bad cast when expecing a class instance") {
    const std::string code = R"(
        import "test" for Vector4

        var V = Vector4.new()

        class Main {
            static main(other) {
                V.from(other)
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<Vector4>("Vector4");
    cls.ctor<>();
    cls.func<&Vector4::from>("from");
    vm.runFromSource("main", code);

    auto main = vm.find("main", "Main").func("main(_)");

    SECTION("Capture correct exception type") {
        REQUIRE_THROWS_AS(main(nullptr), wren::RuntimeError);
    }

    SECTION("Capture correct exception message") {
        REQUIRE_THROWS_WITH(main(nullptr), Catch::Contains("Bad cast when getting value from Wren got null"));
    }
}

class ExceptionClass {
public:
    ExceptionClass() = default;

    void func0Exception() {
        throw std::runtime_error("Something went wrong");
    }

    void func0() {
        // void
    }

    std::string func1Exception() {
        throw std::runtime_error("Something went wrong");
    }

    std::string func1() {
        return "Hello World";
    }

    static void func2Exception() {
        throw std::runtime_error("Something went wrong");
    }

    static void func2() {
        // void
    }

    static std::string func3Exception() {
        throw std::runtime_error("Something went wrong");
    }

    static std::string func3() {
        return "Hello World";
    }
};

struct ExceptionClassHelper {
    static void func0Exception(ExceptionClass& self) {
        throw std::runtime_error("Something went wrong");
    }

    static void func0(ExceptionClass& self) {
        // void
    }

    static std::string func1Exception(ExceptionClass& self) {
        throw std::runtime_error("Something went wrong");
    }

    static std::string func1(ExceptionClass& self) {
        return "Hello World";
    }

    static void func2Exception() {
        throw std::runtime_error("Something went wrong");
    }

    static void func2() {
        // void
    }

    static std::string func3Exception() {
        throw std::runtime_error("Something went wrong");
    }

    static std::string func3() {
        return "Hello World";
    }
};

TEST_CASE("Throw exception in member function") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                var v = ExceptionClass.new()
                v.funcException()
            }
            static main1() {
                var v = ExceptionClass.new()
                v.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.func<&ExceptionClass::func0>("func");
    cls.func<&ExceptionClass::func0Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1());
}

TEST_CASE("Throw exception in member function return type") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                var v = ExceptionClass.new()
                return v.funcException()
            }
            static main1() {
                var v = ExceptionClass.new()
                return v.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.func<&ExceptionClass::func1>("func");
    cls.func<&ExceptionClass::func1Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1().as<std::string>());
}

TEST_CASE("Throw exception in static function") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                ExceptionClass.funcException()
            }
            static main1() {
                ExceptionClass.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.funcStatic<&ExceptionClass::func2>("func");
    cls.funcStatic<&ExceptionClass::func2Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1());
}

TEST_CASE("Throw exception in static function return type") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                return ExceptionClass.funcException()
            }
            static main1() {
                return ExceptionClass.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.funcStatic<&ExceptionClass::func3>("func");
    cls.funcStatic<&ExceptionClass::func3Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1().as<std::string>());
}

TEST_CASE("Throw exception in member function external") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                var v = ExceptionClass.new()
                v.funcException()
            }
            static main1() {
                var v = ExceptionClass.new()
                v.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.funcExt<&ExceptionClassHelper::func0>("func");
    cls.funcExt<&ExceptionClassHelper::func0Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1());
}

TEST_CASE("Throw exception in member function return type external") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                var v = ExceptionClass.new()
                return v.funcException()
            }
            static main1() {
                var v = ExceptionClass.new()
                return v.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.funcExt<&ExceptionClassHelper::func1>("func");
    cls.funcExt<&ExceptionClassHelper::func1Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1().as<std::string>());
}

TEST_CASE("Throw exception in static function external") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                ExceptionClass.funcException()
            }
            static main1() {
                ExceptionClass.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.funcStaticExt<&ExceptionClassHelper::func2>("func");
    cls.funcStaticExt<&ExceptionClassHelper::func2Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1());
}

TEST_CASE("Throw exception in static function return type external") {
    const std::string code = R"(
        import "test" for ExceptionClass

        class Main {
            static main0() {
                return ExceptionClass.funcException()
            }
            static main1() {
                return ExceptionClass.func()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<ExceptionClass>("ExceptionClass");
    cls.ctor<>();
    cls.funcStaticExt<&ExceptionClassHelper::func3>("func");
    cls.funcStaticExt<&ExceptionClassHelper::func3Exception>("funcException");
    vm.runFromSource("main", code);
    auto main0 = vm.find("main", "Main").func("main0()");
    auto main1 = vm.find("main", "Main").func("main1()");

    REQUIRE_THROWS_AS(main0(), wren::RuntimeError);
    REQUIRE_NOTHROW(main1().as<std::string>());
}

TEST_CASE("Push unregistered class") {
    const std::string code = R"(
        class Main {
            static main(instance) {
                return instance.msg
            }
        }
    )";

    wren::VM vm;
    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    REQUIRE_THROWS_AS(main(ExceptionClass()), wren::BadCast);
}
