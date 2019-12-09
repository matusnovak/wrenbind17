---
title: Raw Modules
weight: 110
---

Modules can be created in the following way:

```cpp
wren::VM vm;
auto& m = vm.module("mymodule");

auto& cls = m.klass<Entity>("Entity");
cls.func<&Entity::foo>("foo");
...
```

But, you are also able to create your own "raw" modules.

```cpp
wren::VM vm;
auto& m = vm.module("mymodule");

m.append(R"(
    class Vec3 {
        construct new (x, y, z) {
            ...
        }
    }
)");
```

Anything you add via `m.append(...)` will be loaded into the Wren when you import that specific module. Anything you put inside of `append(...)` will always be appended at the end of the module code. Meaning, if you bind some C++ classes, the raw string(s) you append to that module will always be located after the C++ class foreign code. You can append as much code as you want, until you run out of system memory.

These modules can be loaded in the Wren as the following:

```js
import "mymodule" for Foo, Vec3
```
