---
title: 3. Call Wren function
---

#  3. Call Wren function

## 3.1. Simple call

To call a Wren function, you will have to first run the source code. Only then you can use `wren::VM::find` function to find the class defined in the Wren code. This will give you an object of `wren::Variable` which can be any Wren variable from the VM. In this case, it's a class. Next, find the method you want to call via `wren::Variable::func`. 

You will have to specify the exact signature of the method. In this case, the `static main()` has no parameters, therefore the signature is `main()`. If you have a method with multiple parameters, for example `static main(a, b) { ... }`, then the signature is the following: `main(_,_)`. Use underscores to specify parameters (don't use the parameter name). Whitespace is not allowed.

{{< hint warning >}}
**Warning**

The `wren::VM::find` throws an exception if the class (or a class instance) is not found in the module.
{{< /hint >}}

```cpp
int main(int argc, char *argv[]) {
    const std::string code = R"(
        class Main {
            static main() {
                System.print("Hello World!")
            }
        }
    )";

    // Create new VM
    wren::VM vm;

    // Runs the code from the std::string as a "main" module
    // The System.print("Hello World!") will not be called
    // because it is in a function
    vm.runFromSource("main", code);

    // Find class Main in module main
    wren::Variable mainClass = vm.find("main", "Main");

    // Find function main() in class Main
    wren::Method main = mainClass.func("main()");

    // Execute the function
    // Puts "Hello World!" into the stdout of your terminal.
    // This calls the wren::Method::operator()(...) function.
    main();

    return 0;
}
```

## 3.2. Call with arguments

To call a Wren function that accepts arguments, simply look up the function using a signature that also matches the number of parameters of that function. 

See [4. Supported types]({{< ref "types.md" >}}) to see what types are supported when passing arguments into a Wren function.

{{< hint warning >}}
**Warning**

If you call a Wren function via the `wren::Method::operator()(...)` C++ function, and the number of arguments do not match the signature, it will throw an exception.
{{< /hint >}}

```cpp
int main(int argc, char *argv[]) {
    const std::string code = R"(
        class Main {
            static main(a, b) {
                x = a + b
                System.print("The result is: %(x) ")
            }
        }
    )";

    // Create new VM
    wren::VM vm;

    // Runs the code from the std::string as a "main" module
    // The System.print("Hello World!") will not be called
    // because it is in a function
    vm.runFromSource("main", code);

    // Find class Main in module main
    wren::Variable mainClass = vm.find("main", "Main");

    // Find function main() in class Main
    wren::Method main = mainClass.func("main(_,_)");

    // Execute the function
    // Puts "The result is 25" into the stdout of your terminal.
    // This calls the wren::Method::operator()(...) function.
    main(10, 15);

    return 0;
}
```

## 3.3. Return values

Returning values is simply done via `wren::Any` which can hold any value that Wren can handle. The return value that is held by the `wren::Any` is not guaranteed to be a useable C++ variable. You can call the `wren::Any::is<T>()` function to check if the return value is some C++ type. Due to the design of Wren, the code below will cast the two integers `10` and `15` into doubles (float64) type, because that's how Wren handles numbers (no real integers). 

The `result.is<int>()` below will work because all doubles can be casted into integers, also the other way around. You will loose precision, if you return a double and get it as an integer. **Only numeric types are casted between each other.**

You can always extract the value from `wren::Any` as long as `wren::VM` is alive. Calling the `.as<T>()` function multiple times is allowed.

See [4. Supported types]({{< ref "types.md" >}}) to see what types are supported when returning values from a Wren function.

```cpp
int main(int argc, char *argv[]) {
    const std::string code = R"(
        class Main {
            static main(a, b) {
                return a + b
            }
        }
    )";

    // Create new VM
    wren::VM vm;

    // Runs the code from the std::string as a "main" module
    // The System.print("Hello World!") will not be called
    // because it is in a function
    vm.runFromSource("main", code);

    // Find class Main in module main
    wren::Variable mainClass = vm.find("main", "Main");

    // Find function main() in class Main
    wren::Method main = mainClass.func("main(_,_)");

    // Execute the function
    // This calls the wren::Method::operator()(...) function.
    wren::Any result = main(10, 15);

    assert(result.is<int>());
    std::cout << "The result is: " << result.as<int>() << std::endl;

    return 0;
}
```

## 3.4. Lifetime of return values

The lifetime of the returned value is exteded by the `wren::Any`. You can extend the lifetime of the `wren::Any` beyond the lifetime of `wren::VM`.

Due to the fact that the `wren::Handle` (used by `wren::Any`) is using a weak pointer to the VM, the `wren::Any` becomes empty once the `wren::VM` is destroyed. You can't use it afterwards and any action will result in an exception of type `wren::RuntimeException`. This ensures that if you store `wren::Any` and can't handle the lifetime (for example in a lambda capture) then you won't get segmentation faults. It is handled for you automatically.

## 3.5. Call non-static methods

You can call non-static methods in classes. In all of the examples above, the class has a static main function, but this is not needed. You can do the following:

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

```cpp
// Find the class
wren::Variable wrenClass = vm.find("somemodule", "Instance");

// Find the function with two arguments
wren::Method wrenMethod = wrenClass.func("main(_, _)");
```

{{< hint warning >}}
**Note**

This only works if the instance variable name starts with a capital letter -> `Instance`.
{{< /hint >}}

## 3.6. Call with custom types

You can call Wren function and pass custom C++ types into it. This only works if you have added (binding) your custom type into your Wren VM that you are calling the function in. See [4. Supported types]({{< ref "types.md" >}}) to understand how that is done.


### 3.6.1. Pass by a copy/reference

This is the default behavior of passing class instances to Wren. As shown below, the class will be moved as a copy. Your class must be trivially copyable or a copy constructor defined. Passing anything into a function call will implicitly the use a reference, passing explicitly by a reference also creates a copy. Use a pointer if you do not want to create a copy. Or even better, use a shared pointer.

```cpp
Foo foo = Foo("Hello World");
wren::Method main = vm.find("main", "Main").func("main(_)");
main(foo);
```

### 3.6.2. Pass by a move

It is possible to pass it to Wren by a move as shown below. The instance will be moved into a new `std::shared_ptr<Foo>` handled by the Wren VM. This is possible only if your class has a move constructor.

```cpp
Foo foo = Foo("Hello World");
wren::Method main = vm.find("main", "Main").func("main(_)");
main(std::move(foo));
```

### 3.6.3. Pass by a pointer

The instance will be moved into a new `std::shared_ptr<Foo>` handled by Wren **but the deleter of that shared pointer is empty**. Meaning, you will get the instance inside of Wren, but the lifetime of the instance is determinted solely by the C++. **Make sure the instance you pass as a raw pointer lives longer than the Wren VM**.

```cpp
Foo foo = Foo("Hello World");
wren::Method main = vm.find("main", "Main").func("main(_)");
main(std::move(foo));
```

### 3.6.4. Pass as a shared pointer

**This is the recommended method.** You will not have any issues with the lifetime of the instance. You will also avoid accidental segmentation faults. Moving shared pointer has no different effect than passing shared pointer as a reference or a copy. It will act exactly the same. Doing this will also extend the lifetime of the `Foo` object held by the shared pointer. 

Deletion happens only when Wren garbace collects the instance and the same shared pointer is also no longer needed on C++ side.

TL;DR: Nothing special, it is just a shared pointer.

```cpp
std::shared_ptr<Foo> foo(new Foo("Hello World"));
wren::Method main = vm.find("main", "Main").func("main(_)");
main(foo);
```

## 3.7. Return custom types from Wren

Beware that returning C++ class instances from Wren contains same danger as returning instances from any other C++ function. Returning by a copy is safe, returning by a pointer or a reference is very dangerous, and finally returning by a shared pointer is the safest way to do it.

```cpp
wren::Method method = klass.func("main()");
wren::Any result = method();

Foo result.as<Foo>(); // As a copy

Foo& result.as<Foo&>(); // As a reference
Foo* result.as<Foo*>(); // As a pointer

std::shared_ptr<Foo> result.shared<Foo>(); // As a shared pointer
```

The Wren language is garbage collected. Therefore once the variable is out of the scope, it may or may not be destroyed. It depends on the implementation of the language. But! **The wren::Any will extend the lifetime of the object**, so you can use the "pointer" (or a reference that `.as<T*>()` returns) as long as the result is alive. Once the `wren::Any` falls out of the scope, the pointer or a reference may be invalid because the instance may have been garbage collected.

## 3.8. Catch Wren errors

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

int main(...) {
    wren::VM vm;
    ...

    try {
        wren::Method wrenMain = vm.find("main", "Main").func("main()");
        wrenMain();
    } catch (wren::NotFound& e) {
        // Thows only if class "Main" has not been found
        std::cerr << e.what() << std::endl;
    } catch (wren::RuntimeError& e) {
        // Throw when something went wrong when executing
        // the function. For example: a variable does not
        // exist, or bad method name.
        std::cerr << e.what() << std::endl;
    }

    // If you are lazy, just do the following:
    try {
        wren::Method wrenMain = vm.find("main", "Main").func("main()");
        wrenMain();
    } catch (wren::Exception& e) {
        // catch everything
        std::cerr << e.what() << std::endl;
    }
}
```
