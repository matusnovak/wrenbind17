#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

TEST_CASE("Raw modules") {
    const std::string test = R"(
        class Foo {
            msg {_msg}
            msg=(rhs) {_msg = rhs}

            construct new(msg) {
                this.msg = msg
            }
        }
    )";

    const std::string code = R"(
        import "test" for Foo

        var Instance = Foo.new("Hello World")
        class Main {
            static main() {
                return Instance.msg
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    m.append(test);
    vm.runFromSource("main", code);

    auto main = vm.find("main", "Main").func("main()");
    auto r = main();
    REQUIRE(r.is<std::string>());
    REQUIRE(r.as<std::string>() == "Hello World");
}
