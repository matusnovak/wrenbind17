---
title: Abstract classes
weight: 70
---

What if you want to pass an abstract class to Wren? You can't allocate it, but you can only pass it around as a reference or a pointer? Imagine a specific derived "Entity" class that has a common abstract/interface class?

The only thing you have to do is to NOT to add constructor by calling `ctor`.

```cpp
wren::VM vm;
auto& m = vm.module("mymodule");

// Add class "Vec3"
auto& cls = m.klass<Entity>("Entity");
// cls.ctor<>(); Do not add constructor!
cls.func<&Entity::foo>("foo");
```

