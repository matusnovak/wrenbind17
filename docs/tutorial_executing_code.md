# Tutorial: Alternative ways of executing Wren code

Executing Wren code can be done in the following ways:

```cpp
// Execute from raw std::string and specify the module name manually.
// Can be any module name you want!
vm.runFromSource("main", "var i = 42");

// Same as above, specify the module name but instead
// of the source code you tell it where the file is located.
vm.runFromFile("main", "path/to/some/main.wren");

// Run specific module. This module must be an existing file "utils/libB.wren"
// in one or more look-up paths specified during creation of wren::Vm vm(...);
vm.runFromModule("utils/libB");
```

::: tip
Note!

The `runFromModule` depends on the lookup paths you pass into the `wren::VM` constructor. The name of the module you want to run depends on these paths. Read the [Modularity](tutorial_modularity.md) tutorial first.
:::
