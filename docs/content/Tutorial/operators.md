---
title: 7. Class operators
---

# 7. Class operators

Operators can be added to your custom types, this is done via the `wren::ForeignMethodOperator` enumeration. 

This is a list of all supported operators by WrenBind17:

| Operator | Enum Value |
| -------- | ---------- |
| Add (+)  | OPERATOR_ADD |
| Subtract (-) | OPERATOR_SUB |
| Multiply (*) | OPERATOR_MUL |
| Divide (/) | OPERATOR_DIV |
| Unary negative (-) | OPERATOR_NEG |
| Modulo (%) | OPERATOR_MOD |
| Equal to (==) | OPERATOR_EQUAL |
| Not equal to (!=) | OPERATOR_NOT_EQUAL |
| Greater than (>) | OPERATOR_GT |
| Less than (<) | OPERATOR_LT |
| Greather than or equal (>=) | OPERATOR_GT_EQUAL |
| Less than or equal (<=) | OPERATOR_LT_EQUAL |
| Shift left (<<) | OPERATOR_SHIFT_LEFT |
| Shift right (>>) | OPERATOR_SHIFT_RIGHT |
| Binary and (&) | OPERATOR_AND |
| Binary xor (^) | OPERATOR_XOR |
| Binary or (\|) | OPERATOR_OR |
| Get by index [] | OPERATOR_GET_INDEX |
| Set by index [] | OPERATOR_SET_INDEX |

## 7.1. Basic usage

Adding arithemtic or comparison operators can be done in the following way. 

```cpp
class Vec3 {
public:
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {
    }

    Vec3 operator - () const { // Unary negation operator
        ...
    }

    Vec3 operator + (const Vec3& other) const {
        ...
    }

    Vec3 operator - (const Vec3& other) const {
        ...
    }

    Vec3 operator * (const Vec3& other) const {
        ...
    }

    Vec3 operator / (const Vec3& other) const {
        ...
    }

    bool operator == (const Vec3& other) const {
        ...
    }

    bool operator != (const Vec3& other) const {
        ...
    }

    float x;
    float y;
    float z;
};

int main(...) {
    wren::VM vm;
    ...

    // If you don't do this, the compiler will have no idea
    // which operator to use when binding OPERATOR_SUB and OPERATOR_NEG.
    // With this and static_cast you will explicitly tell the compiler
    // which exact function to use.
    typedef Vec3 (Vec3::*Vec3Sub)(const Vec3&) const;
    typedef Vec3 (Vec3::*Vec3Neg)() const;

    // Bind the class and some basic functions/vars
    auto& cls = m.klass<Vec3>("Vec3");
    cls.ctor<float, float, float>(); // Constructor
    cls.var<&Vec3::x>("x");
    cls.var<&Vec3::y>("y");
    cls.var<&Vec3::z>("z");

    // Bind the operators
    cls.func<&Vec3::operator+ >(wren::OPERATOR_ADD);
    cls.func<static_cast<Vec3Sub>(&Vec3::operator-)>(wren::OPERATOR_SUB);
    cls.func<static_cast<Vec3Neg>(&Vec3::operator-)>(wren::OPERATOR_NEG);
    cls.func<&Vec3::operator* >(wren::OPERATOR_MUL);
    cls.func<&Vec3::operator/ >(wren::OPERATOR_DIV);
    cls.func<&Vec3::operator== >(wren::OPERATOR_EQUAL);
    cls.func<&Vec3::operator!= >(wren::OPERATOR_NOT_EQUAL);
}
```

Afterwards you can use it in the following way:

```js
import "test" for Vec3
var a = Vec3.new(1.0, 2.0, 3.0)
var b = Vec3.new(4.0, 5.0, 6.0)
var c = a + b
```

{{< hint info >}}
**Note**

If you are using Visual Studio and trying to bind operator `<` then you might get an error: `error C2833: 'operator >' is not a recognized operator or type`. This happens because the compiler is unable to understand `cls.func<&Vec3::operator>>(wren::OPERATOR_GT)`. Simply, put `( )` around the operator like this: `cls.func<(&Vec3::operator>)>(...)`.
{{< /hint >}}

{{< hint info >}}
**Note**

Using `*=`, `-=`, `+=`, or `/=` is not allowed. Wren does not support these assignment operators and results in Wren compilation error.
{{< /hint >}}

## 7.2. Operator with multiple types

Consider the following C++ class:

```cpp
class Vec3 {
public:
    Vec3 operator * (const Vec3& other) const {
        ...
    }

    Vec3 operator * (const float value) const {
        ...
    }
};
```

You have two operators but the second one only accepts a single value. This can be useful when you want to, for example, multiply a 3D vector with a constant value. This can be a problem when binding these two operators to Wren. You can't bind them both, but you can use `std::variant<>` instead.

Create a new function in the following way:

```cpp
Vec3 operator * (const std::variant<Vec3, float>& var) const {
    if (var.index() == 0) {
        Vec3 other = std::get<Vec3>(var);
        // Multiply by other vector
    } else {
        float other = std::get<float>(var);
        // Multiply by constant value
    }
}

int main(...) {
    wren::VM vm;
    ...

    auto& cls = m.klass<Vec3>("Vec3");
    cls.ctor<float, float, float>();
    cls.var<&Vec3::x>("x");
    cls.var<&Vec3::y>("y");
    cls.var<&Vec3::z>("z");

    // Optional typedef to explicitly select the correct 
    // operator with the std::variant
    typedef Vec3 (Vec3::*Vec3Mul)(const std::variant<Vec3, float>&) const;
    // Bind the function
    cls.func<static_cast<Vec3Mul>(&Vec3::operator*)>(wren::OPERATOR_MUL);
}
```

Then, insie of Wren, you can do the following:

```js
import "test" for Vec3
var a = Vec3.new(1.0, 2.0, 3.0)
var b = Vec3.new(4.0, 5.0, 6.0)

// Multiply by the other vector
var c = a + b

// Or multiply by a constant value
var c = a * 1.5
```
