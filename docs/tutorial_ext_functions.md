# Tutorial: Binding C++ class methods via external functions

Suppose you have some C++ class you want to bind to Wren, but you can't modify this class because it is from some other library, for example from STL. Or, you want to add some custom behavior but such C++ method does not exist. In this case, you can do the following:

```cpp
// Custom member function with "self" as this pointer
template<typename T>
bool vectorContains(std::vector<T>& self, const T& value) {
    return std::find(self.begin(), self.end(), value) != self.end();
}

// Custom static function without any "self"
template<typename T>
bool vectorFactory(const T& value) {
    std::vector<T> vec;
    vec.push_back(value);
    return vec;
}

auto& cls = m.klass<std::vector<int>>("VectorInt");
cls.ctor<>();
cls.funcExt<&vectorContains<int>>("contains");
cls.funcStaticExt<&vectorFactory<int>>("factory");
```

```js
import "mymodule" for VectorInt

var a = VectorInt.new()
var b = VectorInt.factory(456) // Calls vectorFactory<T>
a.contains(123) // returns bool
```

"Ext" simply means that this is an external function, and the first parameter **must** accept a reference to the class you are binding. However, static functions do not require this. Do not mistake this with "extern" C++ keyword. It has nothing to do with that. (Maybe there is a better word for it?) Additionally, if you look at the `vectorContains` function from above, there is no "this" pointer because this is not a member function. Instead, the "this" is provided as a custom parameter in the first position. This also works with `propExt` and `propReadonlyExt`, and static via `funcStaticExt`.

