#include <limits>
#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class SomeVec3 {
public:
    SomeVec3(float x, float y, float z) : x(x), y(y), z(z) {
    }

    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    SomeVec3 operator - () const {
        SomeVec3 copy = *this;
        copy.x = -copy.x;
        copy.y = -copy.y;
        copy.z = -copy.z;
        return copy;
    }

    // Making it non-const to test templates for both const and non const cases!
    SomeVec3 operator + (const SomeVec3& other) {
        SomeVec3 copy = *this;
        copy.x += other.x;
        copy.y += other.y;
        copy.z += other.z;
        return copy;
    }

    SomeVec3 operator - (const SomeVec3& other) const {
        SomeVec3 copy = *this;
        copy.x -= other.x;
        copy.y -= other.y;
        copy.z -= other.z;
        return copy;
    }

    SomeVec3 operator * (const std::variant<SomeVec3, float>& var) const {
        if (var.index() == 0) {
            SomeVec3 other = std::get<SomeVec3>(var);
            SomeVec3 copy = *this;
            copy.x *= other.x;
            copy.y *= other.y;
            copy.z *= other.z;
            return copy;
        } else {
            float other = std::get<float>(var);
            SomeVec3 copy = *this;
            copy.x *= other;
            copy.y *= other;
            copy.z *= other;
            return copy;
        }
    }

    SomeVec3 operator / (const SomeVec3& other) const {
        SomeVec3 copy = *this;
        copy.x /= other.x;
        copy.y /= other.y;
        copy.z /= other.z;
        return copy;
    }

    bool operator == (const SomeVec3& other) const {
        static constexpr auto epsilon = std::numeric_limits<float>::epsilon();
        return 
            std::abs(x - other.x) < epsilon && 
            std::abs(y - other.y) < epsilon && 
            std::abs(z - other.z) < epsilon;
    }

    bool operator != (const SomeVec3& other) const {
        static constexpr auto epsilon = std::numeric_limits<float>::epsilon();
        return 
            std::abs(x - other.x) > epsilon || 
            std::abs(y - other.y) > epsilon || 
            std::abs(z - other.z) > epsilon;
    }

    bool operator < (const SomeVec3& other) const {
        return length() < other.length();
    }

    bool operator > (const SomeVec3& other) const {
        return length() > other.length();
    }

    bool operator <= (const SomeVec3& other) const {
        return length() <= other.length();
    }

    bool operator >= (const SomeVec3& other) const {
        return length() >= other.length();
    }

    float x;
    float y;
    float z;
};

TEST_CASE("Basic operator binding") {
    REQUIRE(SomeVec3(1.0f, 2.0f, 3.0f) == SomeVec3(1.0f, 2.0f, 3.0f));
    REQUIRE(SomeVec3(1.0f, 2.0f, 3.0f) != SomeVec3(1.0f, 2.0f, 4.0f));

    const std::string code = R"(
        import "test" for Vec3
        class Main {
            static add() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(4.0, 5.0, 6.0)
                return a + b
            }

            static sub() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(4.0, 5.0, 6.0)
                return a - b
            }

            static mul() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(4.0, 5.0, 6.0)
                return a * b
            }

            static div() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(4.0, 4.0, 6.0)
                return a / b
            }

            static neg() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                return -a
            }

            static chain() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(4.0, 4.0, 6.0)
                var c = Vec3.new(0.5, 0.5, 0.5)
                return (a + b) * c
            }

            static mulConst() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                return a * 2.0
            }

            static equals() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(1.0, 2.0, 3.0)
                System.print("Equals? %(a == b) ")
                return a == b
            }

            static notEquals() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(4.0, 4.0, 6.0)
                return a != b
            }

            static gt() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(1.0, 2.0, 3.0)
                return a > b
            }

            static lt() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(1.0, 2.0, 3.0)
                return a < b
            }

            static gtEq() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(1.0, 2.0, 3.0)
                return a >= b
            }

            static ltEq() {
                var a = Vec3.new(1.0, 2.0, 3.0)
                var b = Vec3.new(1.0, 2.0, 3.0)
                return a <= b
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");

    auto& cls = m.klass<SomeVec3>("Vec3");
    cls.ctor<float, float, float>();
    cls.var<&SomeVec3::x>("x");
    cls.var<&SomeVec3::y>("y");
    cls.var<&SomeVec3::z>("z");

    typedef SomeVec3 (SomeVec3::*SomeVec3Sub)(const SomeVec3&) const;
    typedef SomeVec3 (SomeVec3::*SomeVec3Neg)() const;

    cls.func<&SomeVec3::operator+>(wren::OPERATOR_ADD);
    cls.func<static_cast<SomeVec3Sub>(&SomeVec3::operator-)>(wren::OPERATOR_SUB);
    cls.func<static_cast<SomeVec3Neg>(&SomeVec3::operator-)>(wren::OPERATOR_NEG);
    cls.func<&SomeVec3::operator*>(wren::OPERATOR_MUL);
    cls.func<&SomeVec3::operator/>(wren::OPERATOR_DIV);
    cls.func<&SomeVec3::operator==>(wren::OPERATOR_EQUAL);
    cls.func<&SomeVec3::operator!=>(wren::OPERATOR_NOT_EQUAL);
    cls.func<&SomeVec3::operator< >(wren::OPERATOR_LT);
    cls.func<(&SomeVec3::operator>)>(wren::OPERATOR_GT);
    cls.func<&SomeVec3::operator<= >(wren::OPERATOR_LT_EQUAL);
    cls.func<&SomeVec3::operator>= >(wren::OPERATOR_GT_EQUAL);

    vm.runFromSource("main", code);
    auto res = vm.find("main", "Main").func("add()")();
    REQUIRE(res.is<SomeVec3>());
    REQUIRE(res.as<SomeVec3>() == SomeVec3(5.0f, 7.0f, 9.0f));

    res = vm.find("main", "Main").func("sub()")();
    REQUIRE(res.is<SomeVec3>());
    REQUIRE(res.as<SomeVec3>() == SomeVec3(-3.0f, -3.0f, -3.0f));

    res = vm.find("main", "Main").func("mul()")();
    REQUIRE(res.is<SomeVec3>());
    REQUIRE(res.as<SomeVec3>() == SomeVec3(4.0f, 10.0f, 18.0f));

    res = vm.find("main", "Main").func("div()")();
    REQUIRE(res.is<SomeVec3>());
    REQUIRE(res.as<SomeVec3>() == SomeVec3(0.25f, 0.5f, 0.5f));

    res = vm.find("main", "Main").func("neg()")();
    REQUIRE(res.is<SomeVec3>());
    REQUIRE(res.as<SomeVec3>() == SomeVec3(-1.0f, -2.0f, -3.0f));

    res = vm.find("main", "Main").func("chain()")();
    REQUIRE(res.is<SomeVec3>());
    REQUIRE(res.as<SomeVec3>() == SomeVec3(2.5f, 3.0f, 4.5f));

    res = vm.find("main", "Main").func("mulConst()")();
    REQUIRE(res.is<SomeVec3>());
    REQUIRE(res.as<SomeVec3>() == SomeVec3(2.0f, 4.0f, 6.0f));

    res = vm.find("main", "Main").func("equals()")();
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);

    res = vm.find("main", "Main").func("notEquals()")();
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);

    res = vm.find("main", "Main").func("gt()")();
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == false);

    res = vm.find("main", "Main").func("lt()")();
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == false);

    res = vm.find("main", "Main").func("gtEq()")();
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);

    res = vm.find("main", "Main").func("ltEq()")();
    REQUIRE(res.is<bool>());
    REQUIRE(res.as<bool>() == true);
}

class Register {
public:
    Register(const uint32_t value): value(value) {

    }

    Register operator&(const uint32_t value) {
        auto cpy = *this;
        cpy.value &= value;
        return cpy;
    }

    Register operator^(const uint32_t value) {
        auto cpy = *this;
        cpy.value ^= value;
        return cpy;
    }

    Register operator|(const uint32_t value) {
        auto cpy = *this;
        cpy.value |= value;
        return cpy;
    }

    uint32_t value;
};

TEST_CASE("Binary operator binding") {
    const std::string code = R"(
        import "test" for Register
        class Main {
            static and() {
                var reg = Register.new(0xff00)
                return reg & 0x0802
            }

            static xor() {
                var reg = Register.new(0xff00)
                return reg ^ 0x0802
            }

            static or() {
                var reg = Register.new(0xff00)
                return reg | 0x0802
            }
        }
    )";

    wren::VM vm;
    auto& m = vm.module("test");

    auto& cls = m.klass<Register>("Register");
    cls.ctor<uint32_t>();

    cls.func<&Register::operator&>(wren::OPERATOR_AND);
    cls.func<&Register::operator^>(wren::OPERATOR_XOR);
    cls.func<&Register::operator|>(wren::OPERATOR_OR);

    vm.runFromSource("main", code);
    auto res = vm.find("main", "Main").func("and()")();
    REQUIRE(res.is<Register>());
    REQUIRE(res.as<Register>().value == (0xff00 & 0x0802));

    res = vm.find("main", "Main").func("xor()")();
    REQUIRE(res.is<Register>());
    REQUIRE(res.as<Register>().value == (0xff00 ^ 0x0802));

    res = vm.find("main", "Main").func("or()")();
    REQUIRE(res.is<Register>());
    REQUIRE(res.as<Register>().value == (0xff00 | 0x0802));
}
