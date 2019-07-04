# Call Wren and return C++ class

## Example

If the return value is a C++ class, you can return it in different ways. Consider the following Wren example:

```js
import "mymodule" for CppClass

class Foo {
    static baz() {
        return CppClass.new(...)
    }
}
```

On C++ side, you can return it as a copy, pointer, or a shared_ptr:

```cpp
wren::ReturnValue res = baz();

CppClass copy = res.as<CppClass>();
CppClass* ptr = res.as<CppClass*>();
std::shared_ptr<CppClass> sptr = res.shared<CppClass>();
```

Returning as a copy depends on the implementation of the C++ class. If it does not support copying, it will result in a compilation error. However, returning as a pointer does not result in a copy (for obvious reasons). But, there is a catch! What happens with the pointer? Will it get deleted automatically? Well, sort of, [read here about garbage collection and liveness of variables](liveness_garbage.md).
