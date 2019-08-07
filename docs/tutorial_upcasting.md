# Tutorial: Upcasting

Upcasting is when you have a derived class `Enemy` and you would like to upcast it to `Entity`. An `Enemy` class is an `Entity`, but not the other way around. Remember, upcasting is getting the base class!

But, this might be a problem when, for example, you have created a derived class inside of the Wren, and you are passing it into some C++ function that accepts the base class. What you have to do is to tell the Wren what base classes it can be upcasted to. Consider the following example:

```cpp
class Entity {
    void update();
};

class Enemy: public Entity {
    ...
};

class EntityManager {
    void add(std::shared_ptr<Entity> entity) {
        entities.push_back(entity);
    }
};

Wren::VM vm;
auto& m = vm.module("game");

// Class Entity
auto& entityCls = m.klass<Entity>("Entity");
entityCls.func<&Entity::update>("update");

// Class Enemy
// Notice how we are adding two classes here as template parameters.
// The first template parameter is the class you are binding.
// The second and next template parameters are the classes for upcasting.
auto& enemyCls = m.klass<Enemy, Entity>("Enemy");
// Classes won't automatically inherit functions and properties
// therefore you will have to explicitly add them for each
// derived class!
enemyCls.func<&Enemy::update>("update");

// Class EntityManager
auto& mngClass =m.klass<EntityManager>("EntityManager");
mngClass.func<&EntityManager::add>("add");
```

And the Wren code:

```js
import "game" for EntityManager, Enemy

var manager = ...

var e = Enemy.new()
e.update()
manager.add(e) // ok
```

::: tip
Note!

Upcasting such as this only works when you want to accept a reference, pointer, or a shared_ptr of the base class. This won't work with plain value types.
:::
