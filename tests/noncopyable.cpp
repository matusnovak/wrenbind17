#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

static int counter = 0;

class NonCopyable {
public:
    NonCopyable(const std::string& name) : parent(nullptr), name(name) {
        counter++;
        id = size_t(this);
    }
    NonCopyable(const std::string& name, NonCopyable& parent) : parent(&parent), name(name) {
        counter++;
        id = size_t(this);
    }
    NonCopyable(const NonCopyable& other) = delete;
    NonCopyable(NonCopyable&& other) noexcept : parent(nullptr) {
        swap(other);
    }
    NonCopyable& operator=(const NonCopyable& other) = delete;
    NonCopyable& operator=(NonCopyable&& other) noexcept {
        if (this != &other) {
            swap(other);
        }
        return *this;
    }
    ~NonCopyable() {
        counter--;
    }
    void swap(NonCopyable& other) noexcept {
        std::swap(parent, other.parent);
        std::swap(name, other.name);
        std::swap(id, other.id);
    }

    void setParent(NonCopyable& other) {
        parent = &other;
    }

    void setParentPtr(NonCopyable* other) {
        parent = other;
    }

    const std::string& getName() const {
        return name;
    }

    size_t getId() const {
        return id;
    }

    const NonCopyable& getParent() const {
        if (!parent)
            throw std::runtime_error("No parrent assigned");
        return *parent;
    }

    NonCopyable* getParentPtr() const {
        if (!parent)
            throw std::runtime_error("No parrent assigned");
        return parent;
    }

private:
    NonCopyable* parent;
    std::string name;
    size_t id;
};

TEST_CASE("Pass class from C++ to Wren as non copyable") {
    const std::string code = R"(
        import "test" for NonCopyable

        class Main {
            static main(other) {
                System.print("NonCopyable name: %(other.name) id: %(other.id) ")
                return other
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<NonCopyable>("NonCopyable");
    cls.ctor<const std::string&>();
    cls.propReadonly<&NonCopyable::getName>("name");
    cls.propReadonly<&NonCopyable::getId>("id");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    auto instance = NonCopyable("Hello NonCopyable");
    auto r = main(&instance);
    REQUIRE(r.is<NonCopyable*>());
    REQUIRE(r.as<NonCopyable*>() == &instance);
}

TEST_CASE("Pass class from Wren to C++ as non copyable") {
    const std::string code = R"(
        import "test" for NonCopyable

        var Instance = NonCopyable.new("Hello Child")

        class Main {
            static main(other) {
                other.setParent(Instance)
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<NonCopyable>("NonCopyable");
    cls.ctor<const std::string&>();
    cls.propReadonly<&NonCopyable::getName>("name");
    cls.propReadonly<&NonCopyable::getId>("id");
    cls.func<&NonCopyable::setParent>("setParent");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    auto instance = NonCopyable("Hello NonCopyable");
    main(&instance);
    REQUIRE(instance.getParent().getName() == "Hello Child");
}

TEST_CASE("Pass class from Wren to C++ as non copyable via prop") {
    const std::string code = R"(
        import "test" for NonCopyable

        var Instance = NonCopyable.new("Hello Child")

        class Main {
            static main(other) {
                other.parent = Instance
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<NonCopyable>("NonCopyable");
    cls.ctor<const std::string&>();
    cls.propReadonly<&NonCopyable::getName>("name");
    cls.propReadonly<&NonCopyable::getId>("id");
    cls.prop<&NonCopyable::getParentPtr, &NonCopyable::setParentPtr>("parent");

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    auto instance = NonCopyable("Hello NonCopyable");
    main(&instance);
    REQUIRE(instance.getParent().getName() == "Hello Child");
}
