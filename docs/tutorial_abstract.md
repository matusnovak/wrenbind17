# Tutorial: Binding C++ abstract classes

What if you want to pass an abstract class to Wren? You can't allocate it. You can only pass it around. Imagine a specific derived "Entity" class that has a common abstract/interface class?

The only thing you have to do is to NOT do add `ctor`

```cpp
wren::VM vm;
auto& m = vm.module("mymodule");

// Add class "Vec3"
auto& cls = m.klass<Entity>("Entity");
// cls.ctor<>(); Do not add constructor!
cls.func<&Entity::foo>("foo");
```