# Tutorial: Lifetime of objects returned by Wren

## Return global variables

Suppose you have created an instance of a C++ class in Wren as a global variable:

```js
import "mymodule" for Foo

// Starts with capital letter so we can access it inside
// functions and classes
var Instance = Foo.new()

class Main {
    static main() {
        return Instance
    }
}
```

Then if you access the instance via C++:

```cpp
auto& main = vm.find("main", "Main").func("main()");
wren::Any result = main();
Foo cpy = result.as<Foo>();
// cpy is now a copy of Wren's Foo instance.
// The lifetime of cpy or "Instance" is not modified.
```

Then nothing much happens because you are creating a copy of Foo. But what if you access it by a pointer?

```cpp
auto& main = vm.find("main", "Main").func("main()");
wren::Any result = main();
Foo* ptr = result.as<Foo*>();
// ptr points to the same instance as Wren's "Instance" var.

// Using this pointer is OK because the instance will be alive
// until the Wren VM shuts down.
ptr->someMethod();

delete ptr; // Segmentation fault, memory is managed by Wren!
```

Then you have a pointer to the same instance, but when will it get free? Only when Wren VM gets shut doen (VM gets deleted, falls out of the scope, etc). So the pointer you get is valid as long as the VM is alive. **After that, it will get deleted by Wren, so don't free it manually!**

## Return local variables

But what about returning local variables?

```js
import "mymodule" for Foo

class Main {
    static main() {
        return Foo.new()
    }
}
```

Then if you access the instance via C++:

```cpp
auto& main = vm.find("main", "Main").func("main()");
Foo* ptr = null;

{ // Inside of some scope
    wren::Any result = main();
    ptr = result.as<Foo*>();
    ptr->someMethod(); // OK
}

// At this point, the result (wren::Any)
// has been destroyed because it has fallen out of the scope

ptr->someMethod(); // Undefined bahvior
```

The Wren language is garbage collected. Therefore once the variable is out of the scope, it may or may not be destroyed. It depends on the implementation of the language. But! **The wren::Any will extend the lifetime of the object** so you can use the pointer as long as the result is alive.

## Using std::shared_ptr

One way to avoid these bad scenarios is to use modern C++11 features! If you use this:

```cpp
auto& main = vm.find("main", "Main").func("main()");


wren::Any result = main();
std::shared_ptr<Foo> ptr = result.shared<Foo>();

ptr->someMethod(); // OK
```

So now you have extended the lifetime of the Foo instance by using a shared pointer. This shared pointer points to the exact same instance as the internal implementation of this library. The instances are passed around as a shared pointer (even if you pass it as a raw pointer), so the Foo will be alive even after the Wren VM has shut down. The memory will be managed by both C++ and Wren, because all C++ class instances are shared pointers.

If it is still confusing, then you can think of it as:

```cpp
// Let's think of this as wren::Any (result of a function call)
std::shared_ptr<Foo> result;

Foo* ptr = result.get(); // Obviously dangerous
std::shared_ptr<Foo> ptr = result; // Safe
```

::: warning
Warning!

This is all OK until your Foo instance from above will use some additional Wren variables. If it is just a plain C++ class and you have only created it on the Wren side, then you have nothing to worry about. If your C++ class contains some other Wren instances, then it really depends on your own implementation of your application.
:::

## Lifetime of objects passed to C++ functions

So how does all of the above behave on C++ functions? Suppose we have the following:

```cpp
class Foo {
    // Danger! We will get an instance of local
    // Foo variable from Wren. This pointer may be invalid 
    // once you exit this function! The pointer will be 
    // valid only during the execution of this function body.
    void setParent(Foo* other) {

    }
};
```

And then inside of Wren code:

```js
class Main {
    static main(child) {
        child.setParent(Foo.new())
    }
}
```

It will behave exactly same as returning a variable from calling Wren function! The pointer will point to the local Wren instance that may be garbage collected almost right after the call has been made. Don't pass instances like that. Either make the instance a global Wren variable or use smart pointers as shown below:

```cpp
class Foo {
    // Much better and it's safe!
    void setParent(std::shared_ptr<Foo> other) {

    }
};
```

And then inside of Wren code:

```js
class Main {
    static main(child) {
        child.setParent(Foo.new())
    }
}
```

## TL;DR

* Getting `Foo` from Wren is OK.
* Getting `Foo*` from Wren is OK as long as the variable in Wren is global. (Or you somehow ensure that you use the dangling pointer on C++ safely, for example using it only inside of the C++ function being called).
* Getting `Foo&` from Wren is same as `Foo*`.
* Getting `std::shared_ptr<Foo>` is always safe.
