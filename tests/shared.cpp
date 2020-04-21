#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

static int counter = 0;

class SomeClass {
public:
    SomeClass(const std::string& name) : name(name) {
        counter++;
        id = size_t(this);
    }
    SomeClass(const std::string& name, std::shared_ptr<SomeClass> parent) : parent(std::move(parent)), name(name) {
        counter++;
        id = size_t(this);
    }
    SomeClass(const SomeClass& other) = delete;
    SomeClass(SomeClass&& other) = delete;
    SomeClass& operator=(const SomeClass& other) = delete;
    SomeClass& operator=(SomeClass&& other) = delete;
    ~SomeClass() {
        counter--;
    }

    void setParent(const std::shared_ptr<SomeClass>& other) {
        parent = other;
    }

    const std::string& getName() const {
        return name;
    }

    size_t getId() const {
        return id;
    }

    const std::shared_ptr<SomeClass>& getParent() const {
        return parent;
    }

private:
    std::shared_ptr<SomeClass> parent;
    std::string name;
    size_t id;
};

TEST_CASE("shared_ptr") {
    const std::string code = R"(
        import "test" for SomeClass

        class Main {
            static main(other) {
                System.print("SomeClass name: %(other.name) id: %(other.id) ")
                return other
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<SomeClass>("SomeClass");
    cls.ctor<const std::string&>();
    cls.propReadonly<&SomeClass::getName>("name");
    cls.propReadonly<&SomeClass::getId>("id");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    auto other = std::make_shared<SomeClass>("Hello SomeClass");
    auto r = main(other);
    REQUIRE(r.is<SomeClass*>());
    REQUIRE(r.as<SomeClass*>() == other.get());
    REQUIRE(r.shared<SomeClass>() == other);
}

TEST_CASE("shared_ptr pass by move") {
    const std::string code = R"(
        import "test" for SomeClass

        class Main {
            static main(other) {
                System.print("SomeClass name: %(other.name) id: %(other.id) ")
                return other
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<SomeClass>("SomeClass");
    cls.ctor<const std::string&>();
    cls.propReadonly<&SomeClass::getName>("name");
    cls.propReadonly<&SomeClass::getId>("id");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    auto other = std::make_shared<SomeClass>("Hello SomeClass");
    auto copy = other;
    auto r = main(std::move(copy));
    REQUIRE(r.is<SomeClass*>());
    REQUIRE(r.as<SomeClass*>() == other.get());
    REQUIRE(r.shared<SomeClass>() == other);
}

TEST_CASE("shared_ptr pass into constructor") {
    const std::string code = R"(
        import "test" for SomeClass

        class Main {
            static main(parent) {
                var other = SomeClass.new("Hello Child", parent)
                System.print("SomeClass name: %(other.name) id: %(other.id) ")
                System.print("SomeClass parent name: %(other.parent.name) id: %(other.parent.id) ")
                return other
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<SomeClass>("SomeClass");
    cls.ctor<const std::string&, std::shared_ptr<SomeClass>>();
    cls.propReadonly<&SomeClass::getName>("name");
    cls.propReadonly<&SomeClass::getId>("id");
    cls.propReadonly<&SomeClass::getParent>("parent");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    auto parent = std::make_shared<SomeClass>("Hello Parent");
    auto r = main(parent);
    REQUIRE(r.is<SomeClass*>());
    auto ptr = r.shared<SomeClass>();
    REQUIRE(ptr->getName() == "Hello Child");
    REQUIRE(ptr->getParent() == parent);
}

TEST_CASE("shared_ptr as return type from local") {
    const std::string code = R"(
        import "test" for SomeClass

        class Main {
            static main(parent) {
                return SomeClass.new("Hello Child", parent)
            }
        }
    )";

    auto parent = std::make_shared<SomeClass>("Hello Parent");
    REQUIRE(counter == 1);

    {
        wren::VM vm;
        auto& m = vm.module("test");
        auto& cls = m.klass<SomeClass>("SomeClass");
        cls.ctor<const std::string&, std::shared_ptr<SomeClass>>();
        cls.propReadonly<&SomeClass::getName>("name");
        cls.propReadonly<&SomeClass::getId>("id");
        cls.propReadonly<&SomeClass::getParent>("parent");

        vm.runFromSource("main", code);
        auto main = vm.find("main", "Main").func("main(_)");

        REQUIRE(counter == 1);
        auto r = main(parent);
        REQUIRE(counter == 2);

        REQUIRE(r.is<SomeClass*>());
        auto ptr = r.shared<SomeClass>();
        REQUIRE(ptr->getName() == "Hello Child");
        REQUIRE(ptr->getParent() == parent);
    }

    REQUIRE(counter == 1);
}

TEST_CASE("shared_ptr set via property") {
    const std::string code = R"(
        import "test" for SomeClass

        class Main {
            static main(parent) {
                var cls = SomeClass.new("Hello Child")
                cls.parent = parent
                return cls
            }
        }
    )";

    auto parent = std::make_shared<SomeClass>("Hello Parent");
    REQUIRE(counter == 1);

    {
        wren::VM vm;
        auto& m = vm.module("test");
        auto& cls = m.klass<SomeClass>("SomeClass");
        cls.ctor<const std::string&>();
        cls.propReadonly<&SomeClass::getName>("name");
        cls.propReadonly<&SomeClass::getId>("id");
        cls.prop<&SomeClass::getParent, &SomeClass::setParent>("parent");

        vm.runFromSource("main", code);
        auto main = vm.find("main", "Main").func("main(_)");

        REQUIRE(counter == 1);
        auto r = main(parent);
        REQUIRE(counter == 2);

        REQUIRE(r.is<SomeClass*>());
        auto ptr = r.shared<SomeClass>();
        REQUIRE(ptr->getName() == "Hello Child");
        REQUIRE(ptr->getParent() == parent);
    }

    REQUIRE(counter == 1);
}

TEST_CASE("shared_ptr set via property but pass as raw ptr to main") {
    const std::string code = R"(
        import "test" for SomeClass

        class Main {
            static main(parent) {
                var cls = SomeClass.new("Hello Child")
                cls.parent = parent
                return cls
            }
        }
    )";

    auto parent = std::make_shared<SomeClass>("Hello Parent");
    REQUIRE(counter == 1);

    {
        wren::VM vm;
        auto& m = vm.module("test");
        auto& cls = m.klass<SomeClass>("SomeClass");
        cls.ctor<const std::string&>();
        cls.propReadonly<&SomeClass::getName>("name");
        cls.propReadonly<&SomeClass::getId>("id");
        cls.prop<&SomeClass::getParent, &SomeClass::setParent>("parent");

        vm.runFromSource("main", code);
        auto main = vm.find("main", "Main").func("main(_)");

        REQUIRE(counter == 1);
        auto r = main(parent.get());
        REQUIRE(counter == 2);

        REQUIRE(r.is<SomeClass*>());
        auto ptr = r.shared<SomeClass>();
        REQUIRE(ptr->getName() == "Hello Child");
        REQUIRE(ptr->getParent().get() == parent.get());
    }

    REQUIRE(counter == 1);
}
