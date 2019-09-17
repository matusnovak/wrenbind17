#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

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
