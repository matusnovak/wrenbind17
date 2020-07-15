#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class RandomClass {
public:
    RandomClass(std::string msg) : msg(msg) {
    }

    ~RandomClass() = default;

    std::string msg;
};

TEST_CASE("Move VM") {
    const std::string code = R"(
        import "test" for RandomClass

        class Main {
            static main() {
                var v = RandomClass.new("Hello World")
				return v.msg
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<RandomClass>("RandomClass");
    cls.ctor<std::string>();
    cls.var<&RandomClass::msg>("msg");

    vm.runFromSource("main", code);

    auto vm2 = std::move(vm);

    auto main = vm2.find("main", "Main").func("main()");
    auto res = main();
    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == "Hello World");
}
