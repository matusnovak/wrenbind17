---
title: Call C++ and pass variables
weight: 100
---

Calling C++ class methods from Wren is easy. (See [Bind C++ Class]({{< ref "bind-cpp-class.md" >}}) tutorial). But, what if you want to return a C++ class instance from a C++ member function, or the other way around? 

## Return class instance from C++

Consider the following example:

```cpp
class Foo {
public:
    Foo() = default;

    Foo getAsCopy();
    Foo* getAsPointer();
    Foo& getAsReference();
    const Foo& getAsConstReference();
    std::shared_ptr<Foo> getAsShared();
};

int main() {
    ...
    wren::VM vm(...);
    auto& m = vm.module("mymodule");

    auto& cls = m.klass<Foo>("Foo");
    cls.func<&Foo::getAsCopy>("getAsCopy");
    cls.func<&Foo::getAsPointer>("getAsPointer");
    cls.func<&Foo::getAsReference>("getAsReference");
    cls.func<&Foo::getAsConstReference>("getAsConstReference");
    cls.func<&Foo::getAsShared>("getAsShared");
}
```

**All of these methods are valid, and can be used by the Wren. The problem is with the lifetime of the instance.**

### Return by copy

Returning by a copy is allowed as long as the class supports copying. If your class is not copyable, binding `getAsCopy` will cause compilation error. The new class copy will be put into a `std::shared_ptr<Foo>` and will be deleted once Wren's garbage collector removes it.

### Return by a pointer or a reference

This is always allowed, no matter the class. Returning as a pointer or as a reference has exactly the same effect. The class instance will be put into a `std::shared_ptr<Foo>` with no destructor, meaning the instance will **not** be deleted once the Wren's garbage collector removes it. The lifetime of the instance is determined by the C++.

### Return by a shared pointer

This is the most safe way how to pass around class instances. The lifetime of the object is determined by the shared pointer itself. You and Wren will both extend and handle the lifetime. The class will get deleted after Wren's garbage collector removes it **and** your shared pointer on C++ side will be cleared/reset/deleted.

## Pass class instance to C++

Consider the following example:

```cpp
class Foo {
public:
    Foo() = default;

    setAsCopy(Foo foo);
    setAsPointer(Foo* foo);
    setAsReference(Foo& foo);
    setAsConstReference(const Foo& foo);
    setAsShared(const std::shared_ptr<Foo>& foo);
};

int main() {
    ...
    wren::VM vm(...);
    auto& m = vm.module("mymodule");

    auto& cls = m.klass<Foo>("Foo");
    cls.func<&Foo::setAsCopy>("setAsCopy");
    cls.func<&Foo::setAsPointer>("setAsPointer");
    cls.func<&Foo::setAsReference>("setAsReference");
    cls.func<&Foo::setAsConstReference>("setAsConstReference");
    cls.func<&Foo::setAsShared>("setAsShared");
}
```

**All of these methods are valid.**

### Pass by a copy

Passing by copy is self-explanatory. This is only allowed if the class is copyable. Otherwise binding this function will cause compilation error.

### Pass by a reference or a pointer

This is not recommended unless you know exactly what you are doing. The lifetime of the class instance you are passing is determined how the class instance was created in the first place. If the instance was created on Wren, you have no idea when the instance will get deleted, and you have no way of detecting that. **This is OK only if you are only using that pointer/reference locally in the function body**, because the garbage collector will not get executed while Wren is waiting for the function to return. If you are storing the pointer/reference globally or on a class level, you might have problems depending on the instance lifetime. 

### Pass by a shared pointer

This is the most safe method. You will essentially extend the lifetime of the instance. All rules of the shared pointer apply here.
