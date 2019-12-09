---
title: Pass multiple types to C++
weight: 160
---

## Basic example

Using `std::variant` is nothing special. When you pass it into Wren, what happens is that this library will check what type is being held by the variant, and then it will pass it into the Wren code. **The Wren will not get the variant instace, but the value it holds!** For example, passing `std::variant<bool, int>` will push either bool or int into Wren.

The same goes for getting values from Wren as variant. Suppose you have a C++ member function that accepts the variant of `std::variant<bool, int>` then when you call this member function in Wren as `foo.set(true)` or `foo.set(42)` it will work, but `foo.set("Hello")` won't work because the variant does not accepts the string!

```cpp
class Foo {
public:
    Foo() {
        ...
    }

    void baz(const std::variant<bool, std::string>& value) {
        switch (value.index()) {
            case 0: {
                // Is bool!
                const auto std::get<bool>(value);
            }
            case 1: {
                // Is string!
                const auto std::get<std::string>(value);
            }
            default: {
                // This should never happen.
            }
        }
    }
}

wren::VM vm = ...;
auto& m = vm.module(...);
auto& cls = m.klass<Foo>("Foo");
cls.func<&Foo::baz>("baz"); // Nothing special, just like any other functions
```

And then inside of Wren:

```js
import "test" for Foo

var foo = Foo.new()
foo.baz(false) // ok
foo.baz("Hello World") // ok
foo.baz(123.456) // error
```

## Limitations

Passing `std::variant` via **non-const reference** is not allowed. The following will **not** work:

```cpp
class Foo {
public:
    Foo() {
        ...
    }

    void baz(std::variant<bool, std::string>& value) {
        ...
    }
}

wren::VM vm = ...;
auto& m = vm.module(...);
auto& cls = m.klass<Foo>("Foo");
cls.func<&Foo::baz>("baz"); // Nothing special, just like any other functions
```
