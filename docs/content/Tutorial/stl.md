---
title: 11. STL Containers
---

# 11. STL Containers

## 11.1. Optionals

The `std::optional<T>` will be converted into a null or the type that it can hold. This also works when you call C++ function from Wren that accepts `std::optional<T>`. You can either call that function with `null` or with the type `T`.

### 11.1.1. Limitations

Passing `std::optional` via non-const reference is not allowed. Also passing as a pointer or a shared pointer does not work either. Only passing as a plain type (copy) or a const reference is allowed.

## 11.2. Variants

Using `std::variant` is nothing special. When you pass it into Wren, what happens is that this library will check what type is being held by the variant, and then it will pass it into the Wren code. The Wren will not get the variant instace, but the value it holds! For example, passing `std::variant<bool, int>` will push either bool or int into Wren.

The same goes for getting values from Wren as variant. Suppose you have a C++ member function that accepts the variant of `std::variant<bool, int>` then when you call this member function in Wren as `foo.baz(true)` or `foo.baz(42)` it will work, but `foo.baz("Hello")` won't work because the variant does not accept the string!

Consider the following example:

```cpp
class Foo {
public:
    Foo() {
        ...
    }

    void baz(const std::variant<bool, std::string>& value) {
        switch (value.index()) {
            case 0: {
                // Is bool!
                const auto std::get<bool>(value);
            }
            case 1: {
                // Is string!
                const auto std::get<std::string>(value);
            }
            default: {
                // This should never happen.
            }
        }
    }
}

wren::VM vm = ...;
auto& m = vm.module(...);
auto& cls = m.klass<Foo>("Foo");
cls.func<&Foo::baz>("baz"); // Nothing special, just like any other functions
```

And the Wren code:

```js
import "test" for Foo

var foo = Foo.new()
foo.baz(false) // ok
foo.baz("Hello World") // ok
foo.baz(123.456) // error
```

### 11.2.1. Limitations

Passing `std::variant` via non-const reference is not allowed. The following code will not work. Also passing as a pointer or a shared pointer does not work either. Only passing as a plain type (copy) or a const reference is allowed.

```cpp
class Foo {
public:
    Foo() {
        ...
    }

    void baz(std::variant<bool, std::string>& value) {
        ...
    }
}

wren::VM vm = ...;
auto& m = vm.module(...);
auto& cls = m.klass<Foo>("Foo");
cls.func<&Foo::baz>("baz"); // Will not work
```

## 11.3. Sequences

WrenBind17 supports the following sequence containers: `std::vector`, `std::list`, `std::deque`, and `std::set`. By default all of them are converted into native Wren lists. This means that when you pass (or some C++ function returns) any of these containers, **they are converted into Wren lists.** **Any modification to that list in Wren has no effect on the C++ container** passed/returned. Wren lists are not the same object as the STL containers.

However, you can add this container to Wren VM as a foreign class. In that case the instance of the C++ container you pass into Wren will become a foreign class, therefore modifying the "list" (a class in reality) will also modify the C++ container -> they are the same object.

This only works if you pass the container (or return from a C++ function) via a non-cost reference, pointer, or a shared pointer. Passing (or returning) via copy will create a copy of that C++ container. This check (whether to convert it to a native list or as a foreign class instance) happens at the runtime.

Consider this following table.

| Pass/return type | Added as a foreign class | Not added as a foreign class |
| ---------------- | --------------------------- | ------------------------------- |
| Pass by a copy   | Copy of the container and pushed to Wren as a foreign class | Converted to native list |
| Pass by a reference | Pushed to Wren as a foreign class with no copy | BadCast exception |
| Pass by a const reference | Copy of the container and pushed to Wren as a foreign class | Converted to native list |
| Pass by a (const) pointer | Pushed to Wren as a foreign class with no copy | BadCast exception |
| Pass by a shared pointer | Pushed to Wren as a foreign class with no copy | BadCast exception |

### 11.3.1. Native lists

By default all of `std::vector`, `std::list`, `std::deque`, and `std::set` are converted to Wren lists. This also works the other way around -> getting a list either by calling a C++ function that accepts a list or returning a list by calling a Wren function.

For example, you can return this native list into a `std::vector` of `std::variant` type.

```js
class Main {
    static main() {
        return [null, 123, true, "Hello World"]
    }
}
```

```cpp
wren::VM vm;

vm.runFromSource("main", /* code from above */);
auto func = vm.find("main", "Main").func("main()");

auto res = func();
assert(res.isList());
auto vec = res.as<std::vector<std::variant<std::nullptr_t, double, bool, std::string>>>();

assert(vec.size() == 4);
assert(std::get<std::nullptr_t>(vec[0]) == nullptr);
assert(std::get<double>(vec[1]) == 123.0);
assert(std::get<bool>(vec[2]) == true);
assert(std::get<std::string>(vec[3]) == "Hello World");
```

{{< hint info >}}
**Note**

In the above example we are getting `std::vector<std::variant<...>>`. If you add this type as a foreign class, the above example would run in the same way. This is because if you add this type as a foreign class, that only affects pushing that type to Wren. To not to use native lists, you will have to do something like this:

```js
list = VectorOfVariant.new()
list.push(null)
list.push(123)
list.push(true)
list.push("Hello World")
return list
```
{{< /hint >}}


### 11.3.2. Lists as foreign classes

If you wish to add the container of some specific type as a foreign class, you can use the following method to do so:

```cpp
Wren::VM vm;
auto& m = vm.module("std");
wren::StdVectorBindings<int>::bind(m, "VectorInt");
```

The `wren::StdVectorBindings` is just a fancy wrapper that adds functions into the class. I highly recommend going through the `wrenbind17/include/wrenbind17/std.hpp` file to see how exactly this works.

And the usage of that in Wren:

```js
import "std" for VectorInt

var v = VectorInt.new()
v.add(42) // Push new value
v.insert(-1, 20) // Insert at the end
v.contains(42) // returns true
v.pop() // Remove last element and returns it
v.count // Returns the length/size
v.size() // Same as the count
v.clear() // Removes everything
v.removeAt(-2) // Removes the 2nd element from back and returns it
v[0] = 43 // Set specific index (negative indexes not supported!)
System.print("Second: %(v[1])") // Get specific index (no negative indexes!)
for (item in v) { // Supports iteration
    System.print("Item: %(item)") // Prints individual elements
}
```

### 11.3.3. Custom list from scratch


If you read through the [Iterator Protocol](http://wren.io/control-flow.html#the-iterator-protocol) section on official Wren documentation, then you know that you need to implement at least 2 functions:

* `iterate(_)`
* `iteratorValue(_)`

Let's implement all of them for `std::vector<int>`! First we need the `iterate()` function. This function must accept already existing iterator or a null. To do this, we will use `std::variant` and an external function that will be bind to Wren via `funcExt`. 

```cpp
typedef typename std::vector<int>::iterator Iterator;
typedef typename std::vector<int> Vector;

static std::variant<bool, Iterator> iterate(
        Vector& self, // this
        std::variant<std::nullptr_t, Iterator> other
    ) {

    // Check if "other" variant is NOT nullptr
    if (other.index() == 1) {
        // Get the 2nd template, the iterator
        auto it = std::get<Iterator>(other);
        ++it;
        if (it != self.end()) {
            // Return the next position
            return {it};
        }

        // Once we reach the end, we must return false
        return {false};
    } else {
        // No iterator supplied, the variant is null,
        // then return the start of the vector
        return {self.begin()};
    }
}
```

Next, we need the `iteratorValue()` function. This one is very simple:

```cpp
static int iteratorValue(Vector& self, std::shared_ptr<Iterator> other) {
    // You could replace the shared_ptr with a simple copy value.
    // But that depends on you.
    auto& it = *other;
    return *it;
}
```

And then bind it!

```cpp
wren::VM vm;
auto& m = vm.module("std");
auto& cls = m.klass<Vector>("VectorInt");
cls.ctor<>();
cls.funcExt<&iterate>("iterate");
cls.funcExt<&iteratorValue>("iteratorValue");
```

That's all you need to implement your custom list and use it inside of a for loop!

### 11.3.4. Custom lists and operator []

To get the operator [] working, you need two functions to set and to get the index.

```cpp
typedef typename std::vector<int>::iterator Iterator;
typedef typename std::vector<int> Vector;

static void setIndex(Vector& self, size_t index, int value) {
    self[index] = value;
}

static int getIndex(Vector& self, size_t index) {
    return self[index];
}
```

And then bind it!

```cpp
wren::VM vm;
auto& m = vm.module("std");
auto& cls = m.klass<Vector>("VectorInt");
cls.ctor<>();
cls.funcExt<&getIndex>(wren::OPERATOR_GET_INDEX);
cls.funcExt<&setIndex>(wren::OPERATOR_SET_INDEX);
```

The enum values you supply instead of function names will create special bindings for overloading operators. Wren will see the following:

```js
// Autogenerated
class VectorInt {
    ...
    foreign [index]         // Get
    foreign [index]=(other) // Set
}
```

## 11.4. Maps

WrenBind17 supports the following key-value containers: `std::map` and `std::unordered_map`. By default all of them are converted into native Wren maps. This means that when you pass any of these containers, **they are converted into Wren maps.** **Any modification to that map in Wren has no effect on the C++ container** passed. Wren maps are not the same object as the STL containers.

{{< hint danger >}}
**It is not possible to convert a native Wren map into a C++ map.** This is a limitation of the Wren language. However, you can use `wren::Map` that will hold a reference to the Wren native map. You can use this class to retrieve values, remove keys, check if key exists, or get the size of the map.
{{< /hint >}}

You can add the C++ map container to Wren VM as a foreign class. In that case the instance of the C++ container you pass into Wren will become a foreign class, therefore modifying the "map" (a class in reality) will also modify the C++ container -> they are the same object.

This only works if you pass the container via a non-cost reference, pointer, or a shared pointer. Passing (or returning) via copy will create a copy of that C++ container. This check (whether to convert it to a native map or as a foreign class instance) happens at the runtime.

Consider this following table.

| Pass/return type | Added as a foreign class | Not added as a foreign class |
| ---------------- | --------------------------- | ------------------------------- |
| Pass by a copy   | Copy of the container and pushed to Wren as a foreign class | Converted to native map |
| Pass by a reference | Pushed to Wren as a foreign class with no copy | BadCast exception |
| Pass by a const reference | Copy of the container and pushed to Wren as a foreign class | Converted to native map |
| Pass by a (const) pointer | Pushed to Wren as a foreign class with no copy | BadCast exception |
| Pass by a shared pointer | Pushed to Wren as a foreign class with no copy | BadCast exception |

### 11.4.1. Native maps

As mentioned above, it is not possible to get a native map from Wren and convert it into STL container. This is because the Wren low level API does not allow iterating over the map. Therefore, WrenBind17 provides `wren::Map` container that works on top of `wren::Handle` (it is a reference and affects the garbage collector).

You can use `wren::Map` to get values via key, remove keys, check if key exists, or get the size of the entire map. Example code below.

```js
class Main {
    static main() {
        return {
            "first": 42,
            "second": true,
            "third": "Hello World",
            "fourth": null
        }
    }

    static other(map) {
        // Do something with the map
	}
}
```

```cpp
wren::VM vm;

vm.runFromSource("main", code);
auto func = vm.find("main", "Main").func("main()");

auto res = func();
res.is<wren::Map>(); // Returns true
res.isMap(); // Returns true

auto map = res.as<wren::Map>();

map.count(); // Returns 4

map.contains(std::string("first")); // Returns true
map.contains(std::string("fifth")); // Returns false

map.get<int>(std::string("first")); // Returns 42
map.get<bool>(std::string("second")); // Returns true
map.get<std::string>(std::string("third")); // Returns "Hello World"
map.get<std::nullptr_t>(std::string("fourth")); // Returns nullptr
map.get<Foo>(std::string("fourth")); // Throws wren::BadCast

map.erase(std::string("first")); // Returns true
map.erase(std::string("fifth")); // Returns false
map.count(); // 3

map.get<int>(std::string("first")); // Throws wren::NotFound

auto other = vm.find("main", "Main").func("other(_)");

other(map); // Pass the map to some other function
```

### 11.4.2. Maps as foreign classes

If you wish to add the container of some specific type as a foreign class, you can use the following method to do so:

```cpp
Wren::VM vm;
auto& m = vm.module("std");
wren::StdUnorderedMapBindings<std::string, std::string>::bind(m, "MapOfStrings");
```

And the usage of that in Wren:

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
```


### 11.4.3. Maps with variant value type

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


### 11.4.4. Custom maps

To create a custom map you will need to implement the [Iterator Protocol](http://wren.io/control-flow.html#the-iterator-protocol) and the `[]` operator. 

Let's start with the basics, create a wren VM and bind the map (with the key and value types defined!) to the Wren. You can't create a generic map, Wren does not support that. If you want to have different map types with different key types or value types, you will need to do this multiple times. That's what `StdMapBindings<K, T>` and `StdUnorderedMapBindings<K, T>` are designed for.

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

