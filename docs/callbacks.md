# Callbacks

## Using Wren's Fn.new in C++

Let's say you have a GUI button widget and you want to have a specific action when a user clicks on it.

```cpp
class GuiButton {
public:
    GuiButton(std::function<bool(int)> func) : func(std::move(func)) {
    }

    virtual ~GuiButton() {
    }

    bool event(int type) {
        return func(type);
    }
private:
    std::function<bool(int)> func;
};
```

And now you would like to create this class in Wren and add some custom Wren function as the callback. What you can do, similarly as a [workaround for inheritance](inheritance.md), is to create a Wren specific wrapper and accept a class instance (in this case, `Fn` class instance from Wren).

```cpp
class WrenGuiButton: public GuiButton {
public:
    WrenGuiButton(wren::Variable fn) 
      : GuiButton(std::bind(&WrenGuiButton::wrenEvent, this, std::placeholders::_1)) {
        callback = fn.func("call(_)");
    }

private:
    bool wrenEvent(int type) {
        return callback(type).as<bool>();
    }

    wren::Method callback;
};
```

Or using lambdas:

```cpp
class WrenGuiButton: public GuiButton {
public:
    WrenGuiButton(wren::Variable fn) 
      : GuiButton([this](int type) -> bool { return callback(type).as<bool>(); }) {
        callback = fn.func("call(_)");
    }

private:
    wren::Method callback;
};
```

And then bind the class in the following way:

```cpp
wren::VM vm;
auto& m = vm.module("test");
auto& cls = m.klass<WrenGuiButton>("GuiButton");
cls.ctor<wren::Variable>();
```

And you can use it in the following way:

```js
import "test" for GuiButton

class Main {
    static main() {
        var btn = GuiButton.new(Fn.new { |arg|
            // "this" is going to be "Main" class
            return true
        })
        return btn
    }
}
```
