#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

TEST_CASE("Char strings as array") {
    const std::string code = R"(
        class Main {
            static main(arg) {
				return arg
            }
        }
    )";

    wren::VM vm;
    vm.runFromSource("main", code);

    auto main = vm.find("main", "Main").func("main(_)");
    auto res = main("Hello World!");
    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == "Hello World!");
}

TEST_CASE("Char strings as pointer") {
    const std::string code = R"(
        class Main {
            static main(arg) {
				return arg
            }
        }
    )";

    wren::VM vm;
    vm.runFromSource("main", code);

    auto main = vm.find("main", "Main").func("main(_)");
    const char* ptr = "Hello World!";
    auto res = main(ptr);
    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == "Hello World!");
}
