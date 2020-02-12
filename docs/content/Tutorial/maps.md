---
title: Maps
weight: 151
---

## Pre-made STL map and unordered_map bindings

Native Wren maps cannot be accessed within C++. There is some work being done here: [wren-lang/wren#725](https://github.com/wren-lang/wren/pull/725) but at the moment it is not in the release version of Wren. As an alternative, WrenBind17 provides a sample wrapper for `std::map` and `std::unordered_map`. This also includes support for `std::variant` as the map value type. The provided wrappers are `StdMapBindings<K, T>` and `StdUnorderedMapBindings<K, T>` where K is the key type (for example an `std::string`) and the T is the value type which can be anything including a variant.

```cpp
Wren::VM vm;
auto& m = vm.module("std");
wren::StdUnorderedMapBindings<std::string, std::string>::bind(m, "MapOfStrings");
```

And the Wren code:

```js
import "std" for MapOfStrings

var map = MapOfStrings.new()

// Set the value using [] operator
map["hello"] = "world" 

// Access the value
var value = map["hello"]

// Exactly same as in std::map::operator[]
// If the key does not exist, then it is created
// using the default value.
// So "value2" becomes empty string!
var value2 = map["nonexisting_key"]

// Removes value by key and returns the value removed.
// If the key does not exist, the returned value is null.
var removed = map.remove("hello")  

// Check if the key exists
if (map.containsKey("hello")) {
    System.print("Key exists")
}

// Clears the map, removing all elements.
// Same as std::map::clear()
map.clear()

// Get the number of elements in the map.
// Both the function size() and the property count do the same thing.
var total = map.size()
var total = map.count

// Check if the map is empty
if (map.empty()) {
    System.print("There is nothing in the map!")
}

// Iterate over the map.
// The map has an iterator of std::map<K, T>::iterator which
// returns std::pair<K, T> pairs.
// So to access the key you have to use the key property of the pair.
// And the same goes for the value.
// This is exactly the same behavior as iterating over the map in C++
for (pair in map) {
    System.print("Key: %(pair.key) value: %(pair.value)")
}
```

{{% notice note %}}
I highly recommend going through `wrenbind17/include/wrenbind17/std.hpp` to see how exactly this works.
{{% /notice %}}

## Maps with variant value type

Sometimes you need more than one type inside of the map, something like a Json. To do that, you can use the `std::variant` as the map mapped type. An example below.

```cpp
typedef std::variant<int, bool, std::string, std::nullptr_t> Multitype;

class FooClass {
public:
    ...

    void useMap(std::unordered_map<std::string, Multitype>& map) {
        // Get the Multitype value by the key
        auto& multitype = map["string"]

         // Get std::string from the variant.
         // Because std::string is the 3rd template argument
         // of the std::variant Multitype, then we 
         // need to use std::get<index> to access the type!
        auto& str = std::get<2>(multitype);
    }
};

int main() {
    Wren::VM vm;
    auto& m = vm.module("std");
    wren::StdUnorderedMapBindings<std::string, Multitype>::bind(m, "MapOfMultitypes");

    auto& cls = m.klass<FooClass>("FooClass");
    cls.func<&FooClass::useMap>("useMap");

    vm.runFromSource(...);

    return 0;
}

```

And the Wren code for the above map of variants:

```js
import "std" for MapOfMultitypes, FooClass

var map = MapOfMultitypes.new()
map["string"] = "world"
map["int"] = 123456
map["null"] = null // Will become std::nullptr_t
map["boolean"] = true

for (pair in map) {
    System.print("Key: %(pair.key) value: %(pair.value)")
}

// Pass the foo to some custom class
var foo = FooClass.new()
foo.useMap(map)
```


## Custom maps

To create a custom map you will need to implement the [Iterator Protocol](http://wren.io/control-flow.html#the-iterator-protocol) and the `[]` operator. 

Let's start with the basics, create a wren VM and add register the map (with the key and value types defined!) to the Wren. You can't create a generic map, Wren does not support that. If you want to have different map types with different key types or value types, you will need to do this multiple times. That's what `StdMapBindings<K, T>` and `StdUnorderedMapBindings<K, T>` are designed for.

```cpp
typedef std::unordered_map<std::string, int> MapOfInts;

int main() {
    wren::VM vm;
    auto& m = vm.module("mymodule");
    auto& cls = m.klass<MapOfInts>("MapOfInts");
    cls.ctor(); // Empty default constructor

    // ...

    return 0;
}
```

Now, the iterator and iterator value. This is based on the Wren requirements to implement a class that can be iterated. See [Iterator Protocol](http://wren.io/control-flow.html#the-iterator-protocol) for more information.

```cpp
static std::variant<bool, MapOfInts::iterator> iterate(
        MapOfInts& self, 
        std::variant<std::nullptr_t, MapOfInts::iterator> other) {

    // If the variant holds "1" then the value being hold is a MapOfInts::iterator
    if (other.index() == 1) {
        auto it = std::get<MapOfInts::iterator>(other);
        ++it;
        if (it != self.end()) {
            return {it};
        }

        return {false};
    } 
    // Otherwise the variant holds "0" therfore a null
    else {
        return {self.begin()};
    }
}

// The "value_type" is the std::pair<K, T> of the MapOfInts
static MapOfInts::value_type iteratorValue(
        MapOfInts& self, 
        std::shared_ptr<MapOfInts::iterator> other) {

    // This simply returns the iterator value which is the std::pair
    auto& it = *other;
    return *it;
}
```

You will also need these two functions to access the key and the value type of the pairs during iteration.

```cpp
// The "key_type" is the std::string and "mapped_type" is the int
static const MapOfInts::key_type& pairKey(MapOfInts::value_type& pair) {
    return pair.first;
}

static const MapOfInts::mapped_type& pairValue(MapOfInts::value_type& pair) {
    return pair.second;
}
```

Furthemore accessing or setting the values by key using the operator `[]` can be done as the following:

```cpp
static void setIndex(MapOfInts& self, const MapOfInts::key_type& key, MapOfInts::mapped_type value) {
    self[key] = std::move(value);
}

static MapOfInts::mapped_type& getIndex(MapOfInts& self, const MapOfInts::key_type& key) {
    return self[key];
}
```

You will have to then bind these functions above to the MapOfInts class.

```cpp
typedef std::unordered_map<std::string, int> MapOfInts;

int main() {
    wren::VM vm;
    auto& m = vm.module("mymodule");
    auto& cls = m.klass<MapOfInts>("MapOfInts");
    cls.ctor(); // Empty default constructor

    // These two functions must be named exactly like this, Wren requires these names.
    cls.funcExt<&iterate>("iterate");
    cls.funcExt<&iteratorValue>("iteratorValue");

    // These two functions add the operator [] functionality.
    cls.funcExt<&getIndex>(wren::ForeignMethodOperator::OPERATOR_GET_INDEX);
    cls.funcExt<&setIndex>(wren::ForeignMethodOperator::OPERATOR_SET_INDEX);

    // Bind the iterator of this map, without this
    // you cannot pass the iterator between Wren and C++
    auto& iter = m.klass<MapOfInts::iterator>("MapOfIntsIter");
    iter.ctor();

    // Bind the pair of this map too. You need this
    // so you can access the keys and values during iteration.
    auto& pair = m.klass<MapOfInts::value_type>("MapOfIntsPair");
    pair.propReadonlyExt<&pairKey>("key");
    pair.propReadonlyExt<&pairValue>("value");

    return 0;
}
```

Sample Wren usage:

```js
import "mymodule" for MapOfInts

var map = MapOfInts.new()

map["first"] = 123
map["second"] = 456
var second = map["second"]


for (pair in map) {
    System.print("Key: %(pair.key) value: %(pair.value)")
}
```
