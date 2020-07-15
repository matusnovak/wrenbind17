#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

TEST_CASE("Pass empty optional into Wren") {
    const std::string code = R"(
        class Main {
            static main(opt) {
                return opt == null
            }
        }
    )";

    wren::VM vm;

    vm.runFromSource("main", code);
    auto res = vm.find("main", "Main").func("main(_)")(std::optional<int>{std::nullopt});

    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);
}

TEST_CASE("Pass optional into Wren") {
    const std::string code = R"(
        class Main {
            static main(opt) {
                return opt == 123
            }
        }
    )";

    wren::VM vm;

    vm.runFromSource("main", code);
    auto res = vm.find("main", "Main").func("main(_)")(std::optional<int>{123});

    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);
}

class OptionalAcceptor {
public:
    void set(const std::optional<int>& opt) {
        this->opt = opt;
    }

    std::optional<int> opt;
};

TEST_CASE("Pass empty optional into C++") {
    const std::string code = R"(
        import "test" 

        class Main {
            static main(acceptor) {
                acceptor.set(null)
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    auto& cls = m.klass<OptionalAcceptor>("OptionalAcceptor");
    cls.func<&OptionalAcceptor::set>("set");

    OptionalAcceptor instance;

    vm.runFromSource("main", code);
    auto res = vm.find("main", "Main").func("main(_)")(&instance);

    REQUIRE(!instance.opt.has_value());
}

TEST_CASE("Pass optional into C++") {
    const std::string code = R"(
        import "test"

        class Main {
            static main(acceptor) {
                acceptor.set(123)
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    auto& cls = m.klass<OptionalAcceptor>("OptionalAcceptor");
    cls.func<&OptionalAcceptor::set>("set");

    OptionalAcceptor instance;

    vm.runFromSource("main", code);
    auto res = vm.find("main", "Main").func("main(_)")(&instance);

    REQUIRE(instance.opt.has_value());
    REQUIRE(instance.opt.value() == 123);
}

class OptionalFoo {
public:
    OptionalFoo(std::string msg) : msg(std::move(msg)) {
    }

    std::string msg;
};

class OptionalFooAcceptor {
public:
    void set(const std::optional<OptionalFoo>& opt) {
        this->opt = opt;
    }

    std::optional<OptionalFoo> opt;
};

TEST_CASE("Pass optional of class into C++") {
    const std::string code = R"(
        import "test" for OptionalFoo

        class Main {
            static main(acceptor) {
                acceptor.set(OptionalFoo.new("Hello World!" ))
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    auto& cls = m.klass<OptionalFooAcceptor>("OptionalAcceptor");
    cls.func<&OptionalFooAcceptor::set>("set");

    auto& cls2 = m.klass<OptionalFoo>("OptionalFoo");
    cls2.ctor<std::string>();

    OptionalFooAcceptor instance;

    vm.runFromSource("main", code);
    auto res = vm.find("main", "Main").func("main(_)")(&instance);

    REQUIRE(instance.opt.has_value());
    REQUIRE(instance.opt.value().msg == "Hello World!");
}
