# Tutorial: Creating modules and custom raw modules

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

Anything you add via `m.append(...)` will be loaded into Wren when you import that module. Anything you put inside of `append(...)` will always be appended at the end of the module. Meaning, if you bind some C++ classes, the raw string(s) you append to that module will always be after the C++ classes. You can append as much code as you want, until you run out of system memory.

These modules can be loaded in Wren by calling:

```js
import "mymodule" for Foo, Vec3
```

::: tip
Note!

You can use `mymodule/something` as a built in module names. Slashes are allowed.
:::
