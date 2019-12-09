---
title: Call Wren and pass variables
weight: 90
---

## Basics

In order to call Wren functions, you will need to define those functions in a Wren class. The most common way to do that is to declare the function as a static function:

```js
class Main {
    static main(a, b) {
        return a + b
    }
}
```

To find the function, you will first need to compile the source and look for the specific class by its name. When you get the class, you can find the any specific member function inside of that class. To execute the function, call the `operator()(Args&&... args)` method on the `wren::Method`. 

```cpp
wren::VM vm;

// Runs the code as a specific "somemodule" module.
vm.runFromSource("somemodule", "...code from above...");

// Find the class
wren::Variable wrenClass = vm.find("somemodule", "Main");

// Find the function with two arguments
wren::Method wrenMethod = wrenClass.func("main(_, _)");

// Call the function with two integers
wren::Any result = wrenMethod(10, 15);

// Check if the result is some specific C++ type
result.is<int>(); // true

// Cast the result to C++ type
int sum = result.as<int>(); // 15
```

{{% notice info %}}
The lifetime of the object returned by calling Wren methods is determined by the `wren::Any` class. If you return an instance of a class from Wren to C++, and you capture that result as a pointer/reference to the class (i.e. `result.as<Foo*>()` or `result.as<Foo&>()`) then the lifetime of that pointer or reference will depend on the lifetime of `wren::Any`. If you are returning a C++ class created on Wren from Wren into C++ side, and you want to keep the instance for longer time, use `result.shared<Foo>()` which will return `std::shared_ptr<Foo>`. This works because **all C++ classes created on Wren are constructed as a shared pointer**.
{{% /notice %}}

## Wren functions don't need to be static

```js
class Main {
    construct new() {

    }

    main(a, b) {
        return a + b
    }
}

var Instance = Main.new()
```

And then in C++:

```cpp
// Find the class
wren::Variable wrenClass = vm.find("somemodule", "Instance");

// Find the function with two arguments
wren::Method wrenMethod = wrenClass.func("main(_, _)");
```

## Pass class instance to Wren by copy/reference

This is the default behavior of passing class instances to Wren. As shown below, the class will be moved as a copy. Your class must be trivially copyable or a copy constructor defined! Because passing anything into a function call will implicitly the use a reference, passing explicitly by a reference also creates a copy. Use a pointer if you do not want to create a copy.

```cpp
Foo foo = Foo("Hello World");
wren::Method main = vm.find("main", "Main").func("main(_)");
main(foo);
```

## Pass class instance to Wren by move

It is possible to pass it to Wren by move as shown below. The instance will be moved into a new `std::shared_ptr<Foo>` handled by Wren. This is possible only if your class has a move constructor!

```cpp
Foo foo = Foo("Hello World");
wren::Method main = vm.find("main", "Main").func("main(_)");
main(std::move(foo));
```

## Pass class instance to Wren by pointer

The instance will be moved into a new `std::shared_ptr<Foo>` handled by Wren **but the destructor is empty**. Meaning, you will get the instance inside of the Wren, but the lifetime of the instance is determinted solely by the C++. **Make sure the instance you pass as a raw pointer lives longer than the Wren VM**.

```cpp
Foo foo = Foo("Hello World");
wren::Method main = vm.find("main", "Main").func("main(_)");
main(std::move(foo));
```

## Pass class instance to Wren by shared pointer

This is the recommended method. You will not have any issues with the lifetime of the instance. You will also avoid accidental segmentation faults. Moving shared pointer has no different effect than passing it as a reference or a copy.

```cpp
std::shared_ptr<Foo> foo(new Foo("Hello World"));
wren::Method main = vm.find("main", "Main").func("main(_)");
main(foo);
```

## Return class instance from Wren

Beware that returning C++ class instances from Wren contains same danger as returning instances from any other C++ function. Returning by a copy is safe, returning by a pointer or a reference is very dangerous (but, there is an exception), and returning by a shared pointer is the safest way to do it.

```cpp
wren::Method method = klass.func("main()");
wren::Any result = method();

Foo result.as<Foo>(); // As a copy

Foo& result.as<Foo&>(); // As a reference
Foo* result.as<Foo*>(); // As a pointer

std::shared_ptr<Foo> result.shared<Foo>(); // As a shared pointer
```

The Wren language is garbage collected. Therefore once the variable is out of the scope, it may or may not be destroyed. It depends on the implementation of the language. But! **The wren::Any will extend the lifetime of the object** so you can use the pointer as long as the result is alive. Once the wren::Any falls out of the scope, the pointer or a reference may be invalid because the instance may have been garbage collected.
