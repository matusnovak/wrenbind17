#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class AbstractEntity {
public:
    virtual ~AbstractEntity() {
    }
    virtual void tick() = 0;
    virtual void onInteractEvent() = 0;
};

class WrenAbstractEntity : public AbstractEntity {
public:
    WrenAbstractEntity(wren::Variable callbacks) {
        tickFn = callbacks.func("tick(_)");
        onInteractFn = callbacks.func("onInteract(_)");
    }

    virtual ~WrenAbstractEntity() {
    }

    void tick() override {
        tickFn(this);
    }

    void onInteractEvent() override {
        onInteractFn(this);
    }

    void setFoo(std::string value) {
        foo = std::move(value);
    }

    const std::string& getFoo() {
        return foo;
    }

private:
    wren::Method tickFn;
    wren::Method onInteractFn;
    std::string foo;
};

TEST_CASE("Improvised virtual class") {
    const std::string code = R"(
        import "test" for AbstractEntity

        class AbstractEntityCallbacks {
            construct new() {
            }

            tick (entity) {
                entity.foo = "Hello World"
            }

            onInteract (entity) {
                entity.foo = "Hello World 2"
            }
        }

        class Main {
            static main() {
                var e = AbstractEntity.new(AbstractEntityCallbacks.new())
                return e
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<WrenAbstractEntity>("AbstractEntity");
    cls.ctor<wren::Variable>();
    cls.prop<&WrenAbstractEntity::getFoo, &WrenAbstractEntity::setFoo>("foo");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");

    auto res = main();
    REQUIRE(res.is<WrenAbstractEntity>());

    auto ptr = res.shared<WrenAbstractEntity>();

    ptr->tick();
    REQUIRE(ptr->getFoo() == "Hello World");

    ptr->onInteractEvent();
    REQUIRE(ptr->getFoo() == "Hello World 2");
}
