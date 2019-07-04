# Modularity

## Built-in modules

Modules can be created in the following way (more about binding C++ classes [here](bind_cpp_class.md)):

```cpp
wren::VM vm;
auto m = vm.module("mymodule");

auto& cls = m.klass<Entity>("Entity");
cls.func<&Entity::foo>("foo");
...
```

But, you are also able to create your own "raw" modules.

```cpp
wren::VM vm;
auto m = vm.module("mymodule");

m.append(R"(
    class Vec3 {
        construct new (x, y, z) {
            ...
        }
    }
)");
```

These modules can be loaded in Wren by calling:

```js
import "mymodule" for Foo, Vec3
```

::: tip
Note!

You can use `mymodule/something` as a built in module names. Slashes are allowed.
:::

## Files as modules

Files can also be loaded into Wren. But this depends on the lookup paths. When creating an instance of the VM, the first argument is a list of paths to use for lookup. Example below:

```cpp
std::vector<std::string> paths = {
    "some/relative/path",
    "C:/absolute/path"
};
wren::VM vm(paths);
```

::: warning
Warning!

The default value of the VM constructor is `{"./"}`. This means that by default the VM will look for files relative to your working directory! If there is a file `libs/mylib.wren` in your working directory, and in Wren you load it as `import "libs/mylib" for XYZ` then it will load it from your working directory.
:::

::: tip
Note!

You can use `vm.runFromModule("main")` to load a file `main.wren` that is located in one of the paths (`relative/folder`) that you have passed to the VM constructor. If you want to explicitly run a specific file (let's say you have multiple `main.wren` in different paths), you will have to use `vm.runFromFile("relative/folder/main", "path to the real file")`. Or just use `vm.runFromSource("relative/folder/main", "source code")`.
:::


So what happens if I want to load `other.wren` that is located in `some/relative/path`? The VM will first look for internal built-in modules. If you have a module named `other` it will be used, and the VM will not look for files on your system. If the file `other.wren` exists in any of the paths you have passed to the VM constructor, it will be loaded. This is somehow similar to how `import` or `require` works in modern JavaScript or TypeScript with npm modules.

For example, you have `import "utils/other/common" for Tokenizer` statement somewhere in your Wren script. Then the VM will look for `utils/other/common` module in the list of the built-in modules you have specified as `vm.m("mymodule")`. If that fails, it will look inside of the paths you have specified in the VM constructor, and will look for a file `utils/other/common.wren` in all of the paths. Only the first match is used. So the order of the paths in the constructor matters.

If the module is not found, an exception is thrown.

In short, it works like this:

```cpp
// Not real source code, this is just a pseudocode!
void loadModule(const std::string& name) {
    for (const auto& path : paths) {
        const auto test = path + "/" + name + ".wren";

        auto file = open(test);

        if (file.exists()) {
            compile(file.raw());
        } else {
            continue;
        }
    }

    throw NotFound();
};
```

::: warning
Warning!

Relative paths do not work. This is a limitation of Wren. Only absolute paths work. Consider that you have passed `relative/folder` into the VM constructor, and you are running file `relative/folder/main.wren`. Now you want to import `relative/folder/other.wren`, you must use `import "relative/folder/other" for XYZ` inside of the `main.wren`. The `import "other" for XYZ` won't work!
:::
