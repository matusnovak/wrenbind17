# Tutorial: Call Wren from C++

## Basics

In order to call Wren functions, you will need to define those functions in a Wren class. The most common way to do that is to declare the function as a static function:

```js
class Main {
    static main(a, b) {
        return a + b
    }
}
```

To find the function, you will first need to compile the source, and look for the specific class. When you get the class, you can find the any specific function inside of that class. To execute the function, call the `operator()(Args&&... args)` method on the `wren::Method`. 

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

::: warning
Warning!

The lifetime of the object returned by calling Wren methods is determined by the wren::Any. If you return an instance of a class from Wren to C++, and you capture that result as a pointer/reference to the class (i.e. `result.as<Foo*>()` or `result.as<Foo&>()`) then the lifetime of that pointer or reference will depend on the lifetime of wren::Any and how is the class instantiated on Wren (did you return a local variable or a global variable?). 

Read more in the [Lifetime of objects returned by Wren](tutorial_lifetime.md) and [Lifetime of objects passed into Wren](tutorial_lifetime_cpp.md) tutorials.
:::

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

## Pass class instance to Wren by copy

[Make sure you have seen tutorial for binding C++ classes here](tutorial_binding.md). 

This is the default behavior of passing class instances to Wren. As shown below, the class will be moved as a copy. Your class must be trivially copyable or a copy constructor defined!

```cpp
Foo foo = Foo("Hello World")
wren::Method main = vm.find("main", "Main").func("main(_)")
main(foo)
```

## Pass class instance to Wren by move

[Make sure you have seen tutorial for binding C++ classes here](tutorial_binding.md). 

It is possible to pass it to Wren by move as shown below. The instance will be moved into a new `std::shared_ptr<Foo>` handled by Wren. This is possible only if your class has a move constructor!

```cpp
Foo foo = Foo("Hello World")
wren::Method main = vm.find("main", "Main").func("main(_)")
main(std::move(foo))
```
