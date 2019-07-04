# Liveness and garbage collection

## Who manages the memory?

This library is meant to be stupid simple. There is no explicit memory management, and instead it follows this formula:

If you create an instance of class on C++, the C++ controls the liveness of the instance. If you pass it into the Wren, it will not be freed by the garbage collector. 

If you create an instance of class on Wren, let's say a foreign C++ class, the Wren controls the liveness of the instance. But, if you call Wren function and you will get the shared pointer of that instance, you will extend the liveness of that instance via the shared_ptr. 

This is because all class instances are wrapped in a `ForeignObject<T>` class and put into Wren's foreign handle. This wrapper consists of a `shared_ptr<T>`. So, every C++ instance is wrapped into `shared_ptr<T>`. But, it works in the following way:

* You call Wren function and pass C++ instance as a value (no reference, no pointer). The value will be copied into a new instance and put into `shared_ptr<T>`. The copy will be freed when Wren garbage collects it. But you can extend the liveness if you get `shared_ptr<T>` of it (by calling Wren method and returning the instance).
* You call Wren function and pass C++ instance as a reference. The reference will be handled as a pointer, and will be put into `shared_ptr<T>` with no deletion. It will not be deleted when Wren harbage collects this instance on Wren side.
* You call Wren function and pass C++ instance as a pointer. Same scenario with reference, it will be put into `shared_ptr<T>` with no deletion.
* You call Wren function and pass C++ instance as a shared pointer. The liveness will depend on the shared pointer. No extra `shared_ptr<T>` will be created, the wrapper will use the shared pointer you have given it.
* You call Wren function and get the return value as a C++ instance. Then read below [Return values](#Return values) section.

## How do I...

* I want C++ to control liveness -> Pass it to Wren as a pointer or a reference.
* I want Wren to control liveness -> Create the instance on Wren.
* I want both C++ and Wren to control liveness -> Create the instance on Wren and return it to C++ as a shared pointer. Or create the instance on C++ and pass it to Wren as a shared pointer.
* I want C++ to take over the liveness -> Not possible.
* I want Wren to take over the liveness -> Not possible.

## Return values

Everything in Wren is garbage collected, and we have no control over that on the C++ side. When you call a Wren function and return a simple type (integer, float, string, boolean, nullptr) it will automatically create a copy on C++ side inside of the `wren::ReturnValue`. You don't have to worry that an integer will get deleted or something. That would be stupid.

But it gets bit tricky when using C++ classes that you have manually added to Wren. When you return such class, the return value is actually a handle. This handle can be considered as a special pointer inside of Wren. If you get the class as a copy (by calling `as<T>()`), there is no problem. If you get the class as a pointer, the liveliness of the instance depends on the handle (and garbage collection). If you continue using the pointer outside of the scope of the `wren::ReturnValue` you will definitely get into segementation fault. This can be avoided by getting the class as `res.shared<T>()`. This will give you a shared_ptr which is held inside of the Wren handle. This won't create duplicates. The instance will continue to live until the `wren::ReturnValue` has falled out of the scope, all of the `shared_ptr<T>` are destroyed, and the variable on Wren is garbage collected.

Example of what not to do:

```js
import "mymodule" for CppClass

class Foo {
    static baz() {
        return CppClass.new(...)
    }
}
```

And C++ code:

```cpp
void CppClass* getMyClass() {
    wren::Method baz = vm.find("main", "Foo").func("baz()");
    wren::ReturnValue res = baz();
    assert(res.is<CppClass>());

    CppClass* ptr = res.as<CppClass*>();
    // Using ptr here is OK because
    // the return value holds the handle which is still alive
    return ptr;
}

int main(...) {
    ...

    CppClass* ptr = getMyClass();
    // wren::ReturnValue has fallen out of scope at this point.
    // It is not guaranteed that CppClass* ptr will be 
    // pointing to valid memory.
}
```

But this is ok:

```cpp
void std::shared_ptr<CppClass> getMyClass() {
    wren::Method baz = vm.find("main", "Foo").func("baz()");
    wren::ReturnValue res = baz();
    assert(res.is<CppClass>());
    return res.shared<CppClass>();
}

int main(...) {
    ...

    std::shared_ptr<CppClass> ptr = getMyClass();
    // Instance is OK, the liveliness is controlled by this std::shared_ptr
    // and by Wren at the same time. This ptr must be destroyed and Wren's 
    // garbage collector must take a place in order to free this instance.
}
```

::: warning
Warning!

The `wren::Variable`, `wren::Method`, and `wren::ReturnValue` contain handles from Wren that extend the liveness of the variables/methods they hold. You must dispose of them before the VM shuts down. Otherwise the destructor of these classes may cause undefined behavior, even a segmentation fault. Be sure that `wren::VM` is the last thing to be destroyed.
:::

## Why this design?

Because it keeps things simple. If you are trying to do something complex where somehow both C++ and Wren can steal instance liveness control from each other, you are probably doing something wrong. Keep things stupid simple! Avoid unexpected bugs.

