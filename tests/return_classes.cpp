#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class NonMoveableFoo {
public:
    NonMoveableFoo(std::string msg) : msg(std::move(msg)) {
        
    }

    NonMoveableFoo(const NonMoveableFoo& other) = delete;
    NonMoveableFoo(NonMoveableFoo&& other) = delete;
    NonMoveableFoo& operator=(NonMoveableFoo&& other) = delete;
    NonMoveableFoo& operator=(const NonMoveableFoo& other) = delete;

    ~NonMoveableFoo() = default;

    const std::string& getMsg() const {
        return msg;
    }
private:
    std::string msg;
};

class MoveableFoo {
public:
    MoveableFoo(std::string msg) : msg(std::move(msg)) {
    }

    MoveableFoo(const MoveableFoo& other) = delete;
    MoveableFoo(MoveableFoo&& other) noexcept {
        std::swap(msg, other.msg);
    }
    MoveableFoo& operator=(MoveableFoo&& other) noexcept {
        if (this != &other) {
            std::swap(msg, other.msg);
        }
        return *this;
    }
    MoveableFoo& operator=(const MoveableFoo& other) = delete;

    ~MoveableFoo() = default;

    const std::string& getMsg() const {
        return msg;
    }

private:
    std::string msg;
};

class CopyableFoo {
public:
    CopyableFoo(std::string msg) : msg(std::move(msg)) {
    }

    CopyableFoo(const CopyableFoo& other) = default;
    CopyableFoo& operator=(const CopyableFoo& other) = default;

    ~CopyableFoo() = default;

    const std::string& getMsg() const {
        return msg;
    }

private:
    std::string msg;
};

class FooManager {
public:
    FooManager() : nonMoveableFoo("Hello World"), copyableFoo("Hello World") {
        
    }

    NonMoveableFoo& getNonMoveableFoo() {
        return nonMoveableFoo;
    }

    NonMoveableFoo* getNonMoveableFooPtr() {
        return &nonMoveableFoo;
    }

    const NonMoveableFoo* getNonMoveableFooPtrConst() const {
        return &nonMoveableFoo;
    }

    const NonMoveableFoo& getNonMoveableFooConst() const {
        return nonMoveableFoo;
    }

    CopyableFoo& getCopyableFoo() {
        return copyableFoo;
    }

    const CopyableFoo& getCopyableFooConst() const {
        return copyableFoo;
    }

    MoveableFoo getMoveableFoo() const {
        return MoveableFoo("Hello World");
    }

    NonMoveableFoo nonMoveableFoo;
    CopyableFoo copyableFoo;
};

TEST_CASE("Return class from C++ to Wren by reference") {
    const std::string code = R"(
        import "test" for FooManager

        var Manager = FooManager.new()

        class Main {
            static main() {
                return Manager.getNonMoveableFoo()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    { 
        auto& cls = m.klass<NonMoveableFoo>("NonMoveableFoo");
        cls.propReadonly<&NonMoveableFoo::getMsg>("msg");
    }

    {
        auto& cls = m.klass<FooManager>("FooManager");
        cls.ctor<>();
        cls.func<&FooManager::getNonMoveableFoo>("getNonMoveableFoo");
    }

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    auto res = main();
    REQUIRE(res.is<NonMoveableFoo>());
    REQUIRE(res.shared<NonMoveableFoo>()->getMsg() == "Hello World");
}

TEST_CASE("Return class from C++ to Wren by const reference") {
    const std::string code = R"(
        import "test" for FooManager

        var Manager = FooManager.new()

        class Main {
            static main() {
                return Manager.getNonMoveableFooConst()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    {
        auto& cls = m.klass<NonMoveableFoo>("NonMoveableFoo");
        cls.propReadonly<&NonMoveableFoo::getMsg>("msg");
    }

    {
        auto& cls = m.klass<FooManager>("FooManager");
        cls.ctor<>();
        cls.func<&FooManager::getNonMoveableFooConst>("getNonMoveableFooConst");
    }

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    auto res = main();
    REQUIRE(res.is<NonMoveableFoo>());
    REQUIRE(res.shared<NonMoveableFoo>()->getMsg() == "Hello World");
}

TEST_CASE("Return class from C++ to Wren by pointer") {
    const std::string code = R"(
        import "test" for FooManager

        var Manager = FooManager.new()

        class Main {
            static main() {
                return Manager.getNonMoveableFooPtr()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    {
        auto& cls = m.klass<NonMoveableFoo>("NonMoveableFoo");
        cls.propReadonly<&NonMoveableFoo::getMsg>("msg");
    }

    {
        auto& cls = m.klass<FooManager>("FooManager");
        cls.ctor<>();
        cls.func<&FooManager::getNonMoveableFooPtr>("getNonMoveableFooPtr");
    }

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    auto res = main();
    REQUIRE(res.is<NonMoveableFoo>());
    REQUIRE(res.shared<NonMoveableFoo>()->getMsg() == "Hello World");
}

TEST_CASE("Return class from C++ to Wren by const pointer") {
    const std::string code = R"(
        import "test" for FooManager

        var Manager = FooManager.new()

        class Main {
            static main() {
                return Manager.getNonMoveableFooPtr()
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    {
        auto& cls = m.klass<NonMoveableFoo>("NonMoveableFoo");
        cls.propReadonly<&NonMoveableFoo::getMsg>("msg");
    }

    {
        auto& cls = m.klass<FooManager>("FooManager");
        cls.ctor<>();
        cls.func<&FooManager::getNonMoveableFooPtrConst>("getNonMoveableFooPtr");
    }

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main()");
    auto res = main();
    REQUIRE(res.is<NonMoveableFoo>());
    REQUIRE(res.shared<NonMoveableFoo>()->getMsg() == "Hello World");
}

