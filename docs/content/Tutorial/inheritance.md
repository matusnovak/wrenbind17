---
title: Inheritance via composition
weight: 120
---

Wren does not support inheritacne of foreign classes, but there is a workaround. Consider the following C++ class:

```cpp
class Entity {
public:
    Entity() { ... }
    virtual ~Entity() { ... }

    virtual void update() = 0;
};
```

Now we want to have our own class in Wren:

```js
import "game" for Entity

class Enemy is Entity { // Not allowed by Wren :(
    construct new (...) {

    }

    update() {
        // Do something specific for Entity class
    }
}
```

**This does not work.** You can't inherit from a foreign class. But, don't lose hope yet, there is a workaround. First, we need to create a C++ derived class of the base abstract class that overrides the update method. This is necessary so that we can call the proper Wren functions.

```cpp
class WrenEntity: public Entity {
public:
    // Pass the Wren class to the constructor
    WrenEntity(wren::Variable derived) {
        // Find all of the methods you want to "override"
        // The number of arguments (_) or (_, _) does matter!
        updateFn = derived.func("update(_)");
    }

    virtual ~WrenEntity() {

    }

    void update() override {
        // Call the overriden Wren methods from
        // the Wren class whenever you need to.
        // Pass this class as the base class
        updateFn(this);
    }

private:
    // Store the Wren methods as class fields
    wren::Method updateFn;
};

wren::VM vm;
auto& m = vm.module("game");
auto& cls = m.klass<WrenEntity>("Entity");
cls.ctor<wren::Variable>();

vm.runFromSource("main", ...);

// Call the main function (see Wren code below)
auto res = vm.find("main", "Main").func("main()");

// Get the instance with Wren's specific functions
std::shared_ptr<WrenEntity> enemy = res.shared<WrenEntity>();
```

And the following Wren code to be used with the code above:

```js
import "game" for Entity

class Enemy {
    update (self) {
        // self points to the base class!
    }
}

class Main {
    static main () {
        // Pass our custom Enemy class
        // to the base Entity class into the constructor.
        // The base class will call the necessary functions.
        return Entity.new(Enemy.new())
    }
}
```

With the example from above, if the methods are missing in the Wren's `Enemy` class, it will simply throw an exception, because `derived.find(signature)` throws `wren::NotFound` exception if no function with that signature has been found.

