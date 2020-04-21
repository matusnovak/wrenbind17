#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class HelloClass {
public:
    HelloClass() = default;
};

TEST_CASE("Returning bad any must throw correct exception") {
    const std::string code = R"(
        import "test" for HelloClass

        class Main {
            static main() {
                return null
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& c = m.klass<HelloClass>("HelloClass");
    (void)c;

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    SECTION("Return as copy") {
        auto res = main();
        REQUIRE(res.is<HelloClass>() == false);
        REQUIRE_THROWS_AS(res.as<HelloClass>(), wren::BadCast);
    }

    SECTION("Return as reference") {
        auto res = main();
        REQUIRE(res.is<HelloClass>() == false);
        REQUIRE_THROWS_AS(res.as<HelloClass&>(), wren::BadCast);
    }

    SECTION("Return as pointer") {
        auto res = main();
        REQUIRE(res.is<HelloClass>() == false);
        REQUIRE_THROWS_AS(res.as<HelloClass*>(), wren::BadCast);
    }

    SECTION("Return as shared") {
        auto res = main();
        REQUIRE(res.is<HelloClass>() == false);
        REQUIRE_THROWS_AS(res.shared<HelloClass>(), wren::BadCast);
    }

    SECTION("Return as null") {
        auto res = main();
        REQUIRE(res.is<std::nullptr_t>() == true);
        REQUIRE_NOTHROW(res.as<std::nullptr_t>());
    }
}
