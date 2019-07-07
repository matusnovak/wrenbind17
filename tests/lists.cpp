#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

TEST_CASE("Pass std vector to Wren") {
    const std::string code = R"(
        import "test" for VectorInt

        class Main {
            static main(vector) {
                return vector[0] + vector[1] + vector[2]
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    auto& cls = m.klass<std::vector<int>>("VectorInt");
    cls.ctor<>();

    typedef int& (std::vector<int>::*VectorAtFuncPtr)(size_t);
    cls.func<static_cast<VectorAtFuncPtr>(&std::vector<int>::at)>(wren::OPERATOR_GET_INDEX);

    typedef void (std::vector<int>::*VectorAddFuncPtr)(const int&);
    cls.func<static_cast<VectorAddFuncPtr>(&std::vector<int>::push_back)>("add");

    std::cout << m.str() << std::endl;

    vm.runFromSource("main", code);
    auto main = vm.find("main", "Main").func("main(_)");

    std::vector<int> vector = {5, 10, 3};

    auto res = main(vector);
    REQUIRE(res.as<int>() == 18);
}
