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

class SomeBaseClass {
public:
    virtual ~SomeBaseClass() = default;

    virtual void derivedMethod() = 0;
};

class SomeDerivedClass : public SomeBaseClass {
public:
    void derivedMethod() override {
        std::cout << "SomeDerivedClass::derivedMethod() called!" << std::endl;
    }
};

class SomeBaseFactory {
public:
    std::shared_ptr<SomeBaseClass> factory(const std::string& name) {
        if (name == "SomeDerivedClass") {
            return std::make_shared<SomeDerivedClass>();
        }
        throw std::runtime_error("No such class");
    }
};

TEST_CASE("Derived method with shared_ptr") {
    wren::VM vm;
    auto& m = vm.module("test");

    { // Register the base class first
        auto& cls = m.klass<SomeBaseClass>("SomeBaseClass");
        cls.func<&SomeBaseClass::derivedMethod>("derivedMethod");
    }

    { // Register the derived class second, you must register the base methods as well!
        auto& cls = m.klass<SomeDerivedClass, SomeBaseClass>("SomeDerivedClass");
        cls.ctor<>(); // Optional
        cls.func<&SomeDerivedClass::derivedMethod>("derivedMethod");
    }

    { // Factory
        auto& cls = m.klass<SomeBaseFactory>("SomeBaseFactory");
        cls.ctor<>();
        cls.func<&SomeBaseFactory::factory>("factory");
    }

    const std::string code = R"(
        import "test" for SomeBaseFactory

        class Main {
            static main() {
                var factory = SomeBaseFactory.new()
                var instance = factory.factory("SomeDerivedClass")
                instance.derivedMethod()
            }
        }
    )";

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    REQUIRE_NOTHROW(main());
}

class FooBase {
public:
    FooBase() {
        std::cout << "Called FooBase()" << std::endl;
        counter++;
    }
    virtual ~FooBase() {
        std::cout << "Called ~FooBase()" << std::endl;
        counter--;
    };

    static inline size_t counter{0};
};

class FooDerived : public FooBase {
public:
    FooDerived() {
        std::cout << "Called FooDerived()" << std::endl;
        counter++;
    }
    virtual ~FooDerived() {
        std::cout << "Called ~FooDerived()" << std::endl;
        counter--;
    }

    static inline size_t counter{0};
};

class FooFactory {
public:
    static std::shared_ptr<FooBase> factory() {
        return std::make_shared<FooDerived>();
    }
};

TEST_CASE("Destructors tests") {
    auto vm = std::make_unique<wren::VM>();

    auto& m = vm->module("test");
    m.klass<FooBase>("FooBase");

    {
        auto& klass = m.klass<FooFactory>("FooFactory");
        klass.funcStatic<&FooFactory::factory>("factory");
    }

    const std::string code = R"(
        import "test" for FooFactory

        var foo = FooFactory.factory()
    )";

    REQUIRE(FooBase::counter == 0);
    REQUIRE(FooDerived::counter == 0);

    vm->runFromSource("main", code);

    REQUIRE(FooBase::counter == 1);
    REQUIRE(FooDerived::counter == 1);

    vm.reset();

    REQUIRE(FooBase::counter == 0);
    REQUIRE(FooDerived::counter == 0);
}
