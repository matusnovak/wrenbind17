---
title: 8. Modules and files
---

# 8. Modules and files

Wren support modularity (official documentation [here](http://wren.io/modularity.html)), but does not exactly work out of the box. WrenBind17 fills this gap by adding a file load function that works using a list of look-up paths. 

If you are familiar with Python, this is almost the same as the Python home path for loading modules. 

```cpp
std::vector<std::string> paths = {
    "some/relative/path",
    "C:/absolute/path"
};
wren::VM vm(paths);
```

It is highly advised to use absolute paths. You can use relative paths, but they will depend on your current working directory. The default value of the VM constructor is `{"./"}`. This means that by default the VM will look for files relative to your working directory. If there is a file named `./libs/mylib.wren` in your working directory, then you can import that file as `import "libs/mylib" for XYZ`. 

## 8.1. Detailed explanation

Consider the following program file structure:

```
myprogram/
    app.exe
    data/
        main.wren
        libA.wren
        utils/
            libB.wren
```

You have three files: `data/main.wren`, `data/libA.wren`, and `data/utils/libB.wren`. Now, inside of your main, you might have something like this:

```js
// File: data/main.wren

import "libA" for XYZ
```

**This won't work by default.** Because of the default argument for the `wren::VM` constructor is `{"./"}`, the libA is being looked for inside of `myprogram/./<import name>.wren` (assuming the current working directory is `myprogram/`). What you should do is to construct `Wren::VM` in the following way:

```cpp
std::vector<std::string> paths = {
    "C:/programs/myprogram/data"
};
wren::VM vm(paths);
```

Only then the `import "libA" for XYZ` will work correctly.

{{< hint warning >}}
**Warning**

Relative imports do not work due to the design of the Wren! You will have to use absolute paths. So, how do you import `myprogram/data/utils/libB.wren` in `myprogram/data/libA.wren`? You simply use `import "utils/libB" for XYZ` in that file.
{{< /hint >}}

### 8.1.1 Import ambiguity

If you have two Wren source code files named exactly the same, but in two different lookup paths, only the first file will be loaded. The `std::vector<std::string>` of paths you use as the first argument of `wren::VM` constructor is an ordered list of paths. The files are looked for in the order you define it.

### 8.1.2. Module names for custom types

Any module name is permitted as long as it can be a valid Wren string. Slashes such as `import "lib/utils/extras" for tokenize` is allowed, simply create a module as `auto& m = vm.module("lib/utils/extras");`.

## 8.2. Custom import mechanism

You can use your own custom mechanism for handling the imports. This is done by defining your own function of the following type:

```cpp
typedef std::function<std::string(
        const std::vector<std::string>& paths, 
        const std::string& name
    )> LoadFileFn;
```

And using it as this:

```cpp
int main(...) {
    wren::VM vm({"./"});

    const myLoader = [](
        const std::vector<std::string>& paths, 
        const std::string& name) -> std::string {
        
        // "paths" - This list comes from the 
        // first argument of the wren::VM constructor.
        //
        // "name" - The name of the import.

        // Return the source code in this function or throw an exception.
        // For example you can throw wren::NotFound();

        return "";
	};

    vm.setLoadFileFunc(myLoader);
}
```

{{< hint warning >}}
**Warning**

Changing the loader function will also modify the `wren::VM::runFromModule` function. That function depends on the loader. The argument you pass into the `runFromModule` will become the `name` parameter in the loader.
{{< /hint >}}

## 8.3. Raw modules

You can create any number of modules. For example:

```cpp
wren::VM vm;
auto& m = vm.module("mymodule");

m.append(R"(
    class Vec3 {
        construct new (x, y, z) {
            _x = x
            _y = y
            _z = z
        }
    }
)");
```

Then the module can be used as:

```js
import "mymodule" for Vec3
```
