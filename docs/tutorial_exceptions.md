# Tutorial: Handling exceptions

## Exception types

One single base exception: `wrenbind17::Exception` (inherits `std::exception`) with the following derived exceptions:

* `wrenbind17::NotFound` - Throw when trying to find a variable or a static class inside of a Wren via `wrenbind17::VM::find(module, name)`.
* `wrenbind17::BadCast` - When passing values into Wren, for example when trying to pass in a class that has not been registered.
* `wrenbind17::RuntimeError` - When calling a Wren method via `wrenbind17::Method::operator()`.

Example of such runtime error:

```
Runtime error: Vector4 does not implement 'a=(_)'.
  at: main:7
```

## Catch C++ exception in Wren

```cpp
class MyCppClass {
public:
    ...
    void someMethod() {
        throw std::runtime_error("hello");
    }
};
```

```js
var fiber = Fiber.new { 
    var i = MyCppClass.new()
    i.someMethod() // C++ throws "hello"
}

var error = fiber.try()
System.print("Caught error: " + error)
```

## Catch Wren exception in C++

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

## Limitations

Due to the design of Wren, it is impossible to pass through the original C++ exception, therefore only the exception message is kept (by throwing a new exception). If you have a custom exception with some additional fields (let's say a http exception with status code and body) then you are out of luck. You will only get `wrenbind17::RuntimeError`.

Getting the exact cause of an exception thrown in a C++ class constructor is not possible when using `Fiber.new`. This does not affect exceptions thrown in a C++ class methods! For example, a C++ class constructor throws `std::runtime_error("hello")` and you are trying to capture the error with the following code:

```js
var fiber = Fiber.new { 
    var i = Foo.new()
}

var error = fiber.try()
System.print("Caught error: " + error)
return error
```

You will only get:

```
Runtime error: Null does not implement 'init new()'.
```

Why? It's a bug of Wren. The workaround in the implementation of wrenbind17 is to push "Null" as the class instance, so that Wren won't be able to continue.

If you call `Foo.new()` outside of the fibre, then you will get the exact message:

```
Runtime error: hello
```
