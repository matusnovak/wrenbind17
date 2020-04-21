#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class BaseClass {
public:
    virtual ~BaseClass() {
    }

    virtual std::string getStuff() = 0;
};

class DerivedClass : public BaseClass {
public:
    virtual ~DerivedClass() {
    }

    std::string getStuff() override {
        return "Hello World!";
    }
};

class BaseClassReceiver {
public:
    std::string receive(BaseClass* ptr) {
        return ptr->getStuff();
    }
};

TEST_CASE("Inheritance") {
    const std::string code = R"(
        import "test" for BaseClassReceiver, DerivedClass

        var Receiver = BaseClassReceiver.new()
        var Derived = DerivedClass.new()

        class Main {
            static main() {
                return Receiver.receive(Derived)
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<BaseClass>("BaseClass");
    (void)cls;

    auto& cls2 = m.klass<DerivedClass, BaseClass>("DerivedClass");
    cls2.ctor<>();

    auto& cls3 = m.klass<BaseClassReceiver>("BaseClassReceiver");
    cls3.ctor<>();
    cls3.func<&BaseClassReceiver::receive>("receive");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    auto res = main();

    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == "Hello World!");
}

TEST_CASE("Inheritance with shared_ptr") {
    const std::string code = R"(
        import "test" for DerivedClass

        class Main {
            static main() {
                return DerivedClass.new()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<BaseClass>("BaseClass");
    (void)cls;

    auto& cls2 = m.klass<DerivedClass, BaseClass>("DerivedClass");
    cls2.ctor<>();

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    auto res = main();

    REQUIRE(res.is<DerivedClass>());

    auto ptr = res.shared<BaseClass>();
    REQUIRE(ptr->getStuff() == "Hello World!");
}
