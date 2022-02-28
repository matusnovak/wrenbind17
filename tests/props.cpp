#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class Vec3 {
public:
    Vec3() = default;
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {
    }
    ~Vec3() = default;

    void set(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    void setX(double value) {
        x = value;
    }
    double getX() const {
        return x;
    }

    void setY(double value) {
        y = value;
    }
    double getY() const {
        return y;
    }

    void setZ(double value) {
        z = value;
    }
    double getZ() const {
        return z;
    }

    double x = 0.0f;
    double y = 0.0f;
    double z = 0.0f;
};

TEST_CASE("Variables") {
    const std::string code = R"(
        import "test" for Vec3
        var A = Vec3.new(1.1, 2.2, 3.3)
        A.x = 4.4
        A.y = 5.5
        A.z = 6.6
        class Main {
            static main() {
                return A.z
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<Vec3>("Vec3");
    cls.ctor<double, double, double>();
    cls.var<&Vec3::x>("x");
    cls.var<&Vec3::y>("y");
    cls.var<&Vec3::z>("z");

    vm.runFromSource("main", code);
    auto baz = vm.find("main", "Main").func("main()");
    auto r = baz();

    REQUIRE(r.is<double>());
    REQUIRE(r.as<double>() == Approx(6.6));
}

TEST_CASE("Properties") {
    const std::string code = R"(
        import "test" for Vec3
        var A = Vec3.new(1.1, 2.2, 3.3)
        A.x = 4.4
        A.y = 5.5
        A.z = 6.6
        class Main {
            static main() {
                return A.z
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    auto& cls = m.klass<Vec3>("Vec3");
    cls.ctor<double, double, double>();
    cls.prop<&Vec3::getX, &Vec3::setX>("x");
    cls.prop<&Vec3::getY, &Vec3::setY>("y");
    cls.prop<&Vec3::getZ, &Vec3::setZ>("z");

    vm.runFromSource("main", code);
    auto baz = vm.find("main", "Main").func("main()");
    auto r = baz();

    REQUIRE(r.is<double>());
    REQUIRE(r.as<double>() == Approx(6.6));
}

class Entity {
public:
    Entity(std::string name) : name(std::move(name)) {
    }
    virtual ~Entity() = default;
    void setOrigin(Vec3 value) {
        origin = value;
    }
    Vec3 getOrigin() const {
        return origin;
    }

private:
    std::string name;
    Vec3 origin;
};

TEST_CASE("Properties as classes") {
    const std::string code = R"(
        import "test" for Vec3, Entity
        var E = Entity.new("My Entity")
        E.origin = Vec3.new(1.1, 2.2, 3.3)
        class Main {
            static main() {
                return E.origin
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");
    {
        auto& cls = m.klass<Vec3>("Vec3");
        cls.ctor<double, double, double>();
        cls.prop<&Vec3::getX, &Vec3::setX>("x");
        cls.prop<&Vec3::getY, &Vec3::setY>("y");
        cls.prop<&Vec3::getZ, &Vec3::setZ>("z");
    }
    {
        auto& cls = m.klass<Entity>("Entity");
        cls.ctor<std::string>();
        cls.prop<&Entity::getOrigin, &Entity::setOrigin>("origin");
    }

    vm.runFromSource("main", code);
    auto baz = vm.find("main", "Main").func("main()");
    auto r = baz();

    REQUIRE(r.is<Vec3>());
    auto v = r.as<Vec3>();
    REQUIRE(v.z == Approx(3.3));
}

class BaseAsset {
public:
    explicit BaseAsset(std::string name) : name(std::move(name)) {

    }

    void setName(std::string value) {
        name = std::move(value);
    }

    const std::string& getName() const {
        return name;
    }

    std::string getSomeString() {
        return "hello from base class";
    }

    std::string name;
};

class ModelAsset : public BaseAsset {
public:
    explicit ModelAsset(std::string name, std::string path) : BaseAsset(std::move(name)), path(std::move(path)) {

    }

    void setPath(std::string value) {
        path = std::move(value);
    }

    const std::string& getPath() const {
        return path;
    }

    std::string path;
};

TEST_CASE("Properties from base class") {
    const auto bind = [](wren::VM& vm) {
        auto& m = vm.module("test");
        {
            auto& cls = m.klass<BaseAsset>("BaseAsset");
            cls.prop<&BaseAsset::getName, &BaseAsset::setName>("name");
            cls.var<&BaseAsset::name>("rawName");
        }
        {
            auto& cls = m.klass<ModelAsset, BaseAsset>("ModelAsset");
            cls.ctor<std::string, std::string>();
            cls.prop<&BaseAsset::getName, &BaseAsset::setName>("name");
            cls.func<&BaseAsset::getSomeString>("getSomeString");
            cls.prop<&ModelAsset::getPath, &ModelAsset::setPath>("path");
            cls.var<&BaseAsset::name>("rawName");
        }

        ModelAsset asset("some_asset_name", "path_to_file.png");
    };

    SECTION("Call function from base class") {
        const std::string code = R"(
            import "test" for ModelAsset

            class Main {
                static main() {
                    var asset = ModelAsset.new("some_asset_name", "path_to_file.png")
                    return asset.getSomeString()
                }
            }
        )";

        wren::VM vm;
        bind(vm);

        vm.runFromSource("main", code);
        auto main = vm.find("main", "Main").func("main()");
        auto r = main();
        REQUIRE(r.is<std::string>());
        REQUIRE(r.as<std::string>() == "hello from base class");
    }

    SECTION("Return base prop") {
        const std::string code = R"(
            import "test" for ModelAsset

            class Main {
                static main() {
                    var asset = ModelAsset.new("some_asset_name", "path_to_file.png")
                    return asset.name
                }
            }
        )";

        wren::VM vm;
        bind(vm);

        vm.runFromSource("main", code);
        auto main = vm.find("main", "Main").func("main()");
        auto r = main();
        REQUIRE(r.is<std::string>());
        REQUIRE(r.as<std::string>() == "some_asset_name");
    }

    SECTION("Return base var") {
        const std::string code = R"(
            import "test" for ModelAsset

            class Main {
                static main() {
                    var asset = ModelAsset.new("some_asset_name", "path_to_file.png")
                    return asset.rawName
                }
            }
        )";

        wren::VM vm;
        bind(vm);

        vm.runFromSource("main", code);
        auto main = vm.find("main", "Main").func("main()");
        auto r = main();
        REQUIRE(r.is<std::string>());
        REQUIRE(r.as<std::string>() == "some_asset_name");
    }
}

