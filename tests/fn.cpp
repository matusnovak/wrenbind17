#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class GuiButton {
public:
    GuiButton(std::function<bool(int)> func) : func(std::move(func)) {
    }

    virtual ~GuiButton() {
    }

    bool event(int type) {
        return func(type);
    }

private:
    std::function<bool(int)> func;
};

class WrenGuiButton : public GuiButton {
public:
    WrenGuiButton(wren::Variable fn) : GuiButton(std::bind(&WrenGuiButton::wrenEvent, this, std::placeholders::_1)) {
        callback = fn.func("call(_)");
    }

    virtual ~WrenGuiButton() {
    }

private:
    bool wrenEvent(int type) {
        return callback(type).as<bool>();
    }

    wren::Method callback;
};

TEST_CASE("Pass Fn to C++ class") {
    const std::string code = R"(
        import "test" for GuiButton

        class Main {
            static main() {
                var btn = GuiButton.new(Fn.new { |arg|
                    System.print("This: %(this) ")
                    return arg == 42
                })
                return btn
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<WrenGuiButton>("GuiButton");
    cls.ctor<wren::Variable>();

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    auto res = main();
    REQUIRE(res.is<WrenGuiButton>());
    REQUIRE(res.shared<WrenGuiButton>()->event(42) == true);
}
