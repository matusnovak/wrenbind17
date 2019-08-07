# Tutorial: Binding C++ class methods via external functions

Suppose you have some C++ class you want to bind to Wren, but you can't modify this class because it is from some other library, for example from STL. Or, you want to add some custom behavior but such C++ method does not exist. In this case, you can do the following:

```cpp
template<typename T>
bool vectorContains(std::vector<T>& self, const T& value) {
    return std::find(self.begin(), self.end(), value) != self.end();
}

auto& cls = m.klass<std::vector<int>>("VectorInt");
cls.ctor<>();
cls.funcExt<&contains<int>>("contains");
```

```js
import "mymodule" for VectorInt

var v = VectorInt.new()
v.contains(123) // returns bool
```

"Ext" simply means that this is an external function, and the first parameter **must** accept a reference to the class you are binding. Do not mistake this with "extern" C++ keyword. It has nothing to do with that. (Maybe there is a better word for it?) Additionally, if you look at the `vectorContains` function from above, there is no "this" pointer because this is not a member function. Instead, the "this" is provided as a custom parameter in the first position. This also works with `propExt` and `propReadonlyExt`.
