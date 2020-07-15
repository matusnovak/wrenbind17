#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

#define RUN(NAME, VALUE) vm.find("main", "Main").func(NAME).operator()(VALUE).as<int>()

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

            static main12(vector) {
                var sum = 0
                for (value in vector) {
                    sum = sum + value
                }
                return sum
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
    SECTION("main12") {
        std::vector<int> empty;
        REQUIRE(RUN("main12(_)", empty) == 0);
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
            static main12(vector) {
                var sum = 0
                for (value in vector) {
                    sum = sum + value
                }
                return sum
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
    SECTION("main12") {
        std::list<int> empty;
        REQUIRE(RUN("main11(_)", empty) == 0);
    }
}

class NonComparable {
    std::string hello;
    std::shared_ptr<int[]> arr;
};

TEST_CASE("Non comparable list") {
    wren::VM vm;

    auto& m = vm.module("test");
    // Just needs to compile OK
    wren::StdVectorBindings<NonComparable>::bind(m, "NonComparable");
}

typedef std::variant<std::nullptr_t, double, bool, std::string> ListItem;

template <typename Container> class NativeListAcceptor {
public:
    NativeListAcceptor() = default;

    void set(const Container& items) {
        this->items = items;
    }

    void set2(Container items) {
        this->items = items;
    }

    const Container& get() const {
        return items;
    }

    Container get2() const {
        return items;
    }

private:
    Container items;
};

TEST_CASE("Native lists of vector") {
    using Container = std::vector<ListItem>;
    using ListAcceptor = NativeListAcceptor<Container>;

    wren::VM vm;
    auto& m = vm.module("test");
    auto& c = m.klass<ListAcceptor>("NativeListAcceptor");
    c.template ctor<>();
    c.template func<&ListAcceptor::set>("set");
    c.template func<&ListAcceptor::get>("get");
    c.template func<&ListAcceptor::set2>("set2");
    c.template func<&ListAcceptor::get2>("get2");

    SECTION("Get element from native array") {
        const std::string code = R"(
            import "test" for NativeListAcceptor

            class Main {
                static main(list) {
                    var items = list.get()
                    System.print("Item 0: %(items[0]) ")
                    System.print("Item 1: %(items[1]) ")
                    return items[1]
                }
            }
        )";

        ListAcceptor instance;
        Container items;
        items.push_back(nullptr);
        items.push_back(std::string("Hello World"));
        instance.set(items);

        vm.runFromSource("main", code);
        auto func = vm.find("main", "Main").func("main(_)");
        auto res = func(&instance).as<std::string>();

        REQUIRE(res == "Hello World");
    }

    SECTION("Get element from native array return as copy") {
        const std::string code = R"(
            import "test" for NativeListAcceptor

            class Main {
                static main(list) {
                    var items = list.get2()
                    System.print("Item 0: %(items[0]) ")
                    System.print("Item 1: %(items[1]) ")
                    return items[1]
                }
            }
        )";

        ListAcceptor instance;
        Container items;
        items.push_back(nullptr);
        items.push_back(std::string("Hello World"));
        instance.set(items);

        vm.runFromSource("main", code);
        auto func = vm.find("main", "Main").func("main(_)");
        auto res = func(&instance).as<std::string>();

        REQUIRE(res == "Hello World");
    }

    SECTION("Set elements into native array") {
        const std::string code = R"(
            import "test" for NativeListAcceptor

            class Main {
                static main(list) {
                    var items = [null, 123, true, "Hello World"]
                    list.set(items)
                }
            }
        )";

        ListAcceptor instance;

        vm.runFromSource("main", code);
        auto func = vm.find("main", "Main").func("main(_)");
        (void)func(&instance);

        REQUIRE(instance.get().size() == 4);
        REQUIRE(std::get<std::nullptr_t>(instance.get()[0]) == nullptr);
        REQUIRE(std::get<double>(instance.get()[1]) == 123.0);
        REQUIRE(std::get<bool>(instance.get()[2]) == true);
        REQUIRE(std::get<std::string>(instance.get()[3]) == "Hello World");
    }

    SECTION("Set elements into native array pass by copy") {
        const std::string code = R"(
            import "test" for NativeListAcceptor

            class Main {
                static main(list) {
                    var items = [null, 123, true, "Hello World"]
                    list.set2(items)
                }
            }
        )";

        ListAcceptor instance;

        vm.runFromSource("main", code);
        auto func = vm.find("main", "Main").func("main(_)");
        (void)func(&instance);

        REQUIRE(instance.get().size() == 4);
        REQUIRE(std::get<std::nullptr_t>(instance.get()[0]) == nullptr);
        REQUIRE(std::get<double>(instance.get()[1]) == 123.0);
        REQUIRE(std::get<bool>(instance.get()[2]) == true);
        REQUIRE(std::get<std::string>(instance.get()[3]) == "Hello World");
    }
}

TEST_CASE("Get native lists as vector") {
    using Container = std::vector<ListItem>;

    const std::string code = R"(
        class Main {
            static main() {
                return [null, 123, true, "Hello World"]
            }
        }
    )";

    wren::VM vm;

    vm.runFromSource("main", code);
    auto func = vm.find("main", "Main").func("main()");

    auto res = func();
    REQUIRE(res.isList());
    auto vec = res.as<Container>();

    REQUIRE(vec.size() == 4);
    REQUIRE(std::get<std::nullptr_t>(vec[0]) == nullptr);
    REQUIRE(std::get<double>(vec[1]) == 123.0);
    REQUIRE(std::get<bool>(vec[2]) == true);
    REQUIRE(std::get<std::string>(vec[3]) == "Hello World");
}
