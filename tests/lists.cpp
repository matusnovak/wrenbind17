#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

#define RUN(NAME, VALUE) \
    vm.find("main", "Main").func(NAME).operator()(VALUE).as<int>()

TEST_CASE("Pass std vector to Wren") {
    const std::string code = R"(
        import "test" for VectorInt

        class Main {
            static main1(vector) {
                vector[2] = 5
                return vector[0] + vector[1] + vector[2]
            }

            static main2(vector) {
                var sum = 0
                for (value in vector) {
                    sum = sum + value
                }
                return sum
            }

            static main3(vector) {
                return vector.count
            }

            static main4(vector) {
                vector.removeAt(1)
                return vector[0] + vector[1]
            }

            static main5(vector) {
                vector.removeAt(-2)
                return vector[0] + vector[1]
            }

            static main6(vector) {
                vector.insert(1, 20)
                return vector.count
            }

            static main7(vector) {
                vector.insert(1, 20)
                return vector[1]
            }

            static main8(vector) {
                vector.insert(-1, 20)
                return vector[3]
            }

            static main9(vector) {
                vector.insert(-2, 20)
                return vector[1]
            }

            static main10(vector) {
                return vector.pop()
            }

            static main11(vector) {
                vector.clear()
                return vector.count
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    wren::StdVectorBindings<int>::bind(m, "VectorInt");

    vm.runFromSource("main", code);

    std::vector<int> vector = {5, 10, 3};

    SECTION("main1") {
        REQUIRE(RUN("main1(_)", vector) == 20);
    }
    SECTION("main2") {
        REQUIRE(RUN("main2(_)", vector) == 18);
    }
    SECTION("main3") {
        REQUIRE(RUN("main3(_)", vector) == 3);
    }
    SECTION("main4") {
        REQUIRE(RUN("main4(_)", vector) == 8);
    }
    SECTION("main5") {
        REQUIRE(RUN("main5(_)", vector) == 8);
    }
    SECTION("main6") {
        REQUIRE(RUN("main6(_)", vector) == 4);
    }
    SECTION("main7") {
        REQUIRE(RUN("main7(_)", vector) == 20);
    }
    SECTION("main8") {
        REQUIRE(RUN("main8(_)", vector) == 20);
    }
    SECTION("main9") {
        REQUIRE(RUN("main9(_)", vector) == 20);
    }
    SECTION("main10") {
        REQUIRE(RUN("main10(_)", vector) == 3);
    }
    SECTION("main11") {
        REQUIRE(RUN("main11(_)", vector) == 0);
    }
}

TEST_CASE("Pass std list to Wren") {
    const std::string code = R"(
        import "test" for ListInt

        class Main {
            static main1(list) {
                list[2] = 5
                return list[0] + list[1] + list[2]
            }

            static main2(list) {
                var sum = 0
                for (value in list) {
                    System.print("Value: %(value) ")
                    sum = sum + value
                }
                return sum
            }

            static main3(list) {
                return list.count
            }

            static main4(list) {
                list.removeAt(1)
                return list[0] + list[1]
            }

            static main5(list) {
                list.removeAt(-2)
                return list[0] + list[1]
            }

            static main6(list) {
                list.insert(1, 20)
                return list.count
            }

            static main7(list) {
                list.insert(1, 20)
                return list[1]
            }

            static main8(list) {
                list.insert(-1, 20)
                return list[3]
            }

            static main9(list) {
                list.insert(-2, 20)
                return list[1]
            }

            static main10(list) {
                return list.pop()
            }

            static main11(list) {
                list.clear()
                return list.count
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    wren::StdListBindings<int>::bind(m, "ListInt");

    vm.runFromSource("main", code);

    std::list<int> list = {5, 10, 3};
    SECTION("main1") {
        REQUIRE(RUN("main1(_)", list) == 20);
    }
    SECTION("main2") {
        REQUIRE(RUN("main2(_)", list) == 18);
    }
    SECTION("main3") {
        REQUIRE(RUN("main3(_)", list) == 3);
    }
    SECTION("main4") {
        REQUIRE(RUN("main4(_)", list) == 8);
    }
    SECTION("main5") {
        REQUIRE(RUN("main5(_)", list) == 8);
    }
    SECTION("main6") {
        REQUIRE(RUN("main6(_)", list) == 4);
    }
    SECTION("main7") {
        REQUIRE(RUN("main7(_)", list) == 20);
    }
    SECTION("main8") {
        REQUIRE(RUN("main8(_)", list) == 20);
    }
    SECTION("main9") {
        REQUIRE(RUN("main9(_)", list) == 20);
    }
    SECTION("main10") {
        REQUIRE(RUN("main10(_)", list) == 3);
    }
    SECTION("main11") {
        REQUIRE(RUN("main11(_)", list) == 0);
    }
}
