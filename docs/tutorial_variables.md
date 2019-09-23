
# Tutorial: Binding C++ class varialbles

There are two ways how to add C++ class variables to Wren. One is [as variables](#as-variables) and the other way is [as properties](#as-properties). In the end, they will act exactly same in Wren. The only difference is on the C++ side!

## As variables

One way is to have a field and simply bind it to the Wren:

```cpp
struct Vec3 {
    float x = 0;
    float y = 0;
    float z = 0;
};

wren::VM vm;
auto& m = vm.module("mymodule");

// Add class "Vec3"
auto& cls = m.klass<Vec3>("Vec3");
cls.ctor<>();
cls.var<&Vec3::x>("x");
cls.var<&Vec3::y>("y");
cls.var<&Vec3::z>("z");
```

## As properties

Another way is to have a getter and a setter and bind those to the Wren:

```cpp
class Vec3 {
public:
    float getX() const     { return x; }
    void setX(float value) { x = value; }
    float getY() const     { return y; }
    void setY(float value) { y = value; }
    float getZ() const     { return z; }
    void setZ(float value) { z = value; }
private:
    float x = 0;
    float y = 0;
    float z = 0;
};

wren::VM vm;
auto& m = vm.module("mymodule");

// Add class "Vec3"
auto& cls = m.klass<Vec3>("Vec3");
cls.ctor<>();
cls.prop<&Vec3::getX, &Vec3::setX>("x");
cls.prop<&Vec3::getY, &Vec3::setY>("y");
cls.prop<&Vec3::getZ, &Vec3::setZ>("z");
```

## Result

Equivalent wren code for both using `.var<&field>("name")` or `.prop<&getter, &setter>("name")`:

```js
// Autogenerated
foreign class Vec3 {
    construct new () {}
    
    foreign x
    foreign x=(rhs)

    foreign y
    foreign y=(rhs)

    foreign z
    foreign z=(rhs)
}
```

And then simply use it in Wren as:

```js
import "mymodule" for Vec3

var v = Vec3.new()
v.x = 1.23
v.y = 0.0
v.z = 42.42
```

## Read only variables

To bind read-only variables you can use `varReadonly` function. This won't define a Wren setter and therefore the variable can be only read. 

```cpp
class Vec3 {
public:
    Vec3(float x, float y, float z) {...}

    const float x;
    const float y;
    const float z;
};

wren::VM vm;
auto& m = vm.module("mymodule");

// Add class "Vec3"
auto& cls = m.klass<Vec3>("Vec3");
cls.ctor<>();
cls.varReadonly<&Vec3::x>("x");
cls.varReadonly<&Vec3::y>("y");
cls.varReadonly<&Vec3::z>("z");
```

Equivalent wren code:

```js
// Autogenerated
foreign class Vec3 {
    construct new () {}
    
    foreign x

    foreign y

    foreign z
}
```

And then simply use it in Wren as:

```js
import "mymodule" for Vec3

var v = Vec3.new(1.1, 2.3, 3.3)
System.print("X value is: %(v.x)") // ok
v.x = 1.23 // error
```

For read-only properties, you can use `propReadonly` as shown below:

```cpp
// Add class "Vec3"
auto& cls = m.klass<Vec3>("Vec3");
cls.ctor<>();
cls.propReadonly<&Vec3::getX>("x");
cls.propReadonly<&Vec3::getY>("y");
cls.propReadonly<&Vec3::getZ>("z");
```

## Via external functions

Sometimes the property simply does not exist in the C++ class you want to use. So, you somehow need to add this into Wren without changing the original class code. One way to do it is through "external" functions. This is simply a function that is static and **must** accept the first parameter as a reference to the class instance. 

```cpp
static float getVec3X(Vec3& self) {
    return self.x;
}

static float getVec3Y(Vec3& self) { ... }
static float getVec3Z(Vec3& self) { ... }

// Add class "Vec3"
auto& cls = m.klass<Vec3>("Vec3");
cls.ctor<>();
cls.propExtReadonly<&getVec3X>("x");
cls.propExtReadonly<&getVec3Y>("y");
cls.propExtReadonly<&getVec3Z>("z");
```