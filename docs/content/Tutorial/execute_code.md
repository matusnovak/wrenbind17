---
title: 5. Executing from file
---

# 5. Executing from file

Executing Wren code can also be done in the following ways:

```cpp
// Same as above, specify the module name but instead
// of the source code you tell it where the file is located.
vm.runFromFile("main", "path/to/some/main.wren");

// Run specific module. This module must be an existing file "utils/libB.wren"
// in one or more look-up paths specified during creation of wren::Vm vm(...);
vm.runFromModule("utils/libB");
```

{{< hint info >}}
**Note**

The `runFromModule` depends on the lookup paths you pass into the `wren::VM` constructor. The name of the module you want to run depends on these paths. Read the [Modules]({{< ref "modules.md" >}}) tutorial first.
{{% /hint %}}
