#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

#define RUN(NAME, VALUE) vm.find("main", "Main").func(NAME).operator()(VALUE).as<int>()

TEST_CASE("Pass std map to Wren") {
    const std::string code = R"(
        import "test" for MapStringInt, VectorString
        
        class Main {
            static main1(map) {
                map["abcd"] = 5
                return map["hello"] + map["world"] + map["abcd"]
            }

            static main2(map) {
                return map.remove("hello")
            }

            static main3(map) {
                return map.remove("bad_key")
            }

            static main4(map) {
                return map.containsKey("hello")
            }

            static main5(map) {
                return map.containsKey("bad_key")
            }

            static main6(map) {
                map["test"] = 456
                return map.count
            }

            static main7(map) {
                return map.empty()
            }

            static main8(map) {
                map.clear()
            }

            static main9(map) {
                map["abcd"] = 5
                var ret = 0
                for (pair in map) {
                    System.print("map pair key: %(pair.key) value: %(pair.value) ")
                    ret = ret + pair.value
                }
                return ret
            }

            static main10(map) {
                map["abcd"] = 5
                var ret = VectorString.new()
                for (pair in map) {
                    ret.add(pair.key)
                }
                return ret
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    wren::StdMapBindings<std::string, int>::bind(m, "MapStringInt");
    wren::StdVectorBindings<std::string>::bind(m, "VectorString");

    vm.runFromSource("main", code);
    auto var = vm.find("main", "Main");

    std::map<std::string, int> map;
    map["hello"] = 123;
    map["world"] = 42;

    SECTION("main1") {
        auto res = var.func("main1(_)")(map);
        REQUIRE(res.is<int>());
        REQUIRE(res.as<int>() == 123 + 42 + 5);
    }

    SECTION("main2") {
        auto res = var.func("main2(_)")(map);
        REQUIRE(res.is<int>());
        REQUIRE(res.as<int>() == 123);
    }

    SECTION("main3") {
        auto res = var.func("main3(_)")(map);
        REQUIRE(res.is<std::nullptr_t>());
    }

    SECTION("main4") {
        auto res = var.func("main4(_)")(map);
        REQUIRE(res.is<bool>());
        REQUIRE(res.as<bool>() == true);
    }

    SECTION("main5") {
        auto res = var.func("main5(_)")(map);
        REQUIRE(res.is<bool>());
        REQUIRE(res.as<bool>() == false);
    }

    SECTION("main6") {
        auto res = var.func("main6(_)")(map);
        REQUIRE(res.is<int>());
        REQUIRE(res.as<int>() == 3);
    }

    SECTION("main7") {
        auto res = var.func("main7(_)")(map);
        REQUIRE(res.is<bool>());
        REQUIRE(res.as<bool>() == false);
        REQUIRE(!map.empty());
    }

    SECTION("main7b") {
        map.clear();
        auto res = var.func("main7(_)")(map);
        REQUIRE(res.is<bool>());
        REQUIRE(res.as<bool>() == true);
        REQUIRE(map.empty());
    }

    SECTION("main8") {
        auto res = var.func("main8(_)")(map);
        REQUIRE(!map.empty());
    }

    SECTION("main8b") {
        // Passing as a pointer -> no copy is made to the map
        auto res = var.func("main8(_)")(&map);
        REQUIRE(map.empty());
    }

    SECTION("main9") {
        auto res = var.func("main9(_)")(map);
        REQUIRE(res.is<int>());
        REQUIRE(res.as<int>() == 123 + 42 + 5);
    }

    SECTION("main10") {
        auto res = var.func("main10(_)")(map);
        REQUIRE(res.is<std::vector<std::string>>());
        auto keys = res.as<std::vector<std::string>>();
        REQUIRE(keys.size() == 3);
        REQUIRE(std::find(keys.begin(), keys.end(), "hello") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "world") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "abcd") != keys.end());
    }
}

class MapValue {
public:
    MapValue() : code(0) {
    }

    MapValue(std::string message, int code) : message(std::move(message)), code(code) {
    }

    const std::string& getMessage() const {
        return message;
    }

    int getCode() const {
        return code;
    }

private:
    std::string message;
    int code;
};

TEST_CASE("Pass std map to Wren with value as a class") {
    const std::string code = R"(
        import "test" for MapIntCustom, MapValue
        
        class Main {
            static main1(map) {
                return map[42]
            }

            static main2(map) {
                map[123] = MapValue.new("World", 987)
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    wren::StdUnorderedMapBindings<int, MapValue>::bind(m, "MapIntCustom");

    auto& cls = m.klass<MapValue>("MapValue");
    cls.ctor<std::string, int>();
    cls.propReadonly<&MapValue::getMessage>("message");
    cls.propReadonly<&MapValue::getCode>("code");

    vm.runFromSource("main", code);
    auto var = vm.find("main", "Main");

    std::unordered_map<int, MapValue> map;
    map[42] = MapValue("Hello", 42);

    SECTION("main1") {
        auto res = var.func("main1(_)")(map);
        REQUIRE(res.is<MapValue>());
        REQUIRE(res.as<MapValue>().getMessage() == "Hello");
    }

    SECTION("main2") {
        var.func("main2(_)")(&map);
        REQUIRE(map.size() == 2);
        REQUIRE(map.at(123).getMessage() == "World");
        REQUIRE(map.at(123).getCode() == 987);
    }
}

TEST_CASE("Pass std map to Wren with variant") {
    const std::string code = R"(
        import "test" for MapMultivalue
        
        class Main {
            static main1(map, key) {
                return map[key]
            }

            static main2(map, key, value) {
                map[key] = value
            }
        }
    )";

    wren::VM vm;

    auto& m = vm.module("test");
    typedef std::variant<int, bool, std::string, std::nullptr_t> Multivalue;
    wren::StdUnorderedMapBindings<std::string, Multivalue>::bind(m, "MapMultivalue");

    vm.runFromSource("main", code);
    auto var = vm.find("main", "Main");

    std::unordered_map<std::string, Multivalue> map;
    map["first"] = 42;
    map["second"] = true;
    map["third"] = std::string("Hello World");
    map["fourth"] = nullptr;

    SECTION("main1") {
        auto func = var.func("main1(_,_)");
        auto res = func(map, std::string("first"));
        REQUIRE(res.is<int>());
        REQUIRE(res.as<int>() == 42);

        res = func(map, std::string("second"));
        REQUIRE(res.is<bool>());
        REQUIRE(res.as<bool>() == true);

        res = func(map, std::string("third"));
        REQUIRE(res.is<std::string>());
        REQUIRE(res.as<std::string>() == std::string("Hello World"));

        res = func(map, std::string("fourth"));
        REQUIRE(res.is<std::nullptr_t>());
    }

    SECTION("main2") {
        auto func = var.func("main2(_,_,_)");
        func(&map, std::string("fifth"), 42);
        REQUIRE(std::get<0>(map.at("fifth")) == 42);

        func(&map, std::string("sixth"), false);
        REQUIRE(std::get<1>(map.at("sixth")) == false);

        func(&map, std::string("seventh"), std::string("test"));
        REQUIRE(std::get<2>(map.at("seventh")) == std::string("test"));

        func(&map, std::string("eighth"), nullptr);
        REQUIRE(std::get<3>(map.at("eighth")) == nullptr);
    }
}

TEST_CASE("Pass std map to Wren as native") {
    const std::string code = R"(
        class Main {
            static main(map, key) {
                return map[key]
            }
        }
    )";

    wren::VM vm;

    vm.runFromSource("main", code);
    auto func = vm.find("main", "Main").func("main(_,_)");

    typedef std::variant<int, bool, std::string, std::nullptr_t> Multivalue;
    std::map<std::string, Multivalue> map;
    map["first"] = 42;
    map["second"] = true;
    map["third"] = std::string("Hello World");
    map["fourth"] = nullptr;

    auto res = func(map, std::string("first"));
    REQUIRE(res.is<int>());
    REQUIRE(res.as<int>() == 42);

    res = func(map, std::string("second"));
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);

    res = func(map, std::string("third"));
    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == std::string("Hello World"));

    res = func(map, std::string("fourth"));
    REQUIRE(res.is<std::nullptr_t>());
}

TEST_CASE("Pass std unordered map to Wren as native") {
    const std::string code = R"(
        class Main {
            static main(map, key) {
                return map[key]
            }
        }
    )";

    wren::VM vm;

    vm.runFromSource("main", code);
    auto func = vm.find("main", "Main").func("main(_,_)");

    typedef std::variant<int, bool, std::string, std::nullptr_t> Multivalue;
    std::unordered_map<std::string, Multivalue> map;
    map["first"] = 42;
    map["second"] = true;
    map["third"] = std::string("Hello World");
    map["fourth"] = nullptr;

    auto res = func(map, std::string("first"));
    REQUIRE(res.is<int>());
    REQUIRE(res.as<int>() == 42);

    res = func(map, std::string("second"));
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);

    res = func(map, std::string("third"));
    REQUIRE(res.is<std::string>());
    REQUIRE(res.as<std::string>() == std::string("Hello World"));

    res = func(map, std::string("fourth"));
    REQUIRE(res.is<std::nullptr_t>());
}

TEST_CASE("Get map to Wren as native") {
    const std::string code = R"(
        class Main {
            static main() {
                return {
                    "first": 42,
                    "second": true,
                    "third": "Hello World",
                    "fourth": null
                }
            }
            static other(map) {
                return map["third"]
            }
        }
    )";

    wren::VM vm;

    vm.runFromSource("main", code);
    auto func = vm.find("main", "Main").func("main()");

    auto res = func();
    REQUIRE(res.is<wren::Map>());
    auto map = res.as<wren::Map>();

    REQUIRE(map.count() == 4);

    REQUIRE(map.contains(std::string("first")) == true);
    REQUIRE(map.contains(std::string("second")) == true);
    REQUIRE(map.contains(std::string("third")) == true);
    REQUIRE(map.contains(std::string("fourth")) == true);
    REQUIRE(map.contains(std::string("fifth")) == false);

    REQUIRE(map.get<int>(std::string("first")) == 42);
    REQUIRE(map.get<bool>(std::string("second")) == true);
    REQUIRE(map.get<std::string>(std::string("third")) == std::string("Hello World"));
    REQUIRE(map.get<std::nullptr_t>(std::string("fourth")) == nullptr);

    REQUIRE(map.erase(std::string("first")) == true);
    REQUIRE(map.erase(std::string("fifth")) == false);
    REQUIRE(map.count() == 3);

    REQUIRE_THROWS(map.get<int>(std::string("first")));

    auto other = vm.find("main", "Main").func("other(_)");
    res = other(map);

    REQUIRE(res.is<std::string>());
}

TEST_CASE("Get map of ints to Wren as native") {
    const std::string code = R"(
        class Main {
            static main() {
                return {
                    0: 42,
                    1: true,
                    2: "Hello World",
                    3: null
                }
            }
            static other(map) {
                return map[2]
            }
        }
    )";

    wren::VM vm;

    vm.runFromSource("main", code);
    auto func = vm.find("main", "Main").func("main()");

    auto res = func();
    REQUIRE(res.is<wren::Map>());
    auto map = res.as<wren::Map>();

    REQUIRE(map.count() == 4);

    REQUIRE(map.contains(0) == true);
    REQUIRE(map.contains(1) == true);
    REQUIRE(map.contains(2) == true);
    REQUIRE(map.contains(3) == true);
    REQUIRE(map.contains(4) == false);

    REQUIRE(map.get<int>(0) == 42);
    REQUIRE(map.get<bool>(1) == true);
    REQUIRE(map.get<std::string>(2) == std::string("Hello World"));
    REQUIRE(map.get<std::nullptr_t>(3) == nullptr);

    REQUIRE(map.erase(0) == true);
    REQUIRE(map.erase(4) == false);
    REQUIRE(map.count() == 3);

    REQUIRE_THROWS(map.get<int>(1));

    auto other = vm.find("main", "Main").func("other(_)");
    res = other(map);

    REQUIRE(res.is<std::string>());
}
