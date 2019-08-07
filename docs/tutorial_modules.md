# Tutorial: External files and modularity

Other source code files can be imported into Wren, but this depends on the lookup paths. When creating an instance of the VM, the first argument is a list of paths to use for lookup. Example below:

```cpp
std::vector<std::string> paths = {
    "some/relative/path",
    "C:/absolute/path"
};
wren::VM vm(paths);
```

::: warning
Warning!

The default value of the VM constructor is `{"./"}`. This means that by default the VM will look for files relative to your working directory! If there is a file `libs/mylib.wren` in your working directory, and in the Wren you load it as `import "libs/mylib" for XYZ` then it will load it from your working directory as `"your_working_dir" + "./" + "libs/mylib.wren"`.
:::

So how does the lookup paths work? Consider the following example:

```
myprogram/
    app.exe
    data/
        main.wren
        libA.wren
        utils/
            libB.wren
```

So you have three files: `data/main.wren`, `data/libA.wren`, and `data/utils/libB.wren`. Now, inside of your main you might have something like this:

```js
// File: data/main.wren

import "libA" for XYZ
// Won't work because "libA.wren" does not exist in "./"
```

And let's assume you are running `app.exe` with working directory of `myprogram/`. The above code will fail. Why? Because the default argument for `wren::VM` paths is `{"./"}`. Meaning, it will look for `libA.wren` relatively to `./` which is relative to your working directory. 

::: warning
Relative imports do not work due to the design of Wren! Absolute imports are the only way!
:::

What you need to do is to put an absolute path to the `myprogram/data` folder into your `wren::VM vm({"C:/whatever/apps/myprogram/data"});`. Only then the import mechanism will work. The import system works in a way that it will append the import string into your lookup paths and checks wether the file exists. If the file exists, it will load it, ignoring other files. This also means that if you have multiple files.

What about importing `utils/libB.wren`? The following will work:

```js
// File: data/libA.wren

import "utils/libB" for XYZ
// looks for "C:/whatever/apps/myprogram/data/" + "utils/libB.wren"
```

Imports from a parent folder work too, because everything is absolute path anyway.

```js
// File: data/utils/libB.wren

import "libA" for XYZ 
// looks for "C:/whatever/apps/myprogram/data/" + "libA.wren"
```

## File ambiguity

Consider files: `test/lib/A.wren` and `main/lib/A.wren`. Putting lookup paths for `test` and `main` folders and then loading it as `import "lib/A"` will load the `lib/A.wren` from the first folder the file exists in. The order of the lookup paths does matter! 
