#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class MyFoo {
public:
    MyFoo(const int year, const std::string& message) : message(message), year(year), type("MyFoo") {
    }

    std::string message;

    const std::string& getType() const {
        return type;
    }

    int getYear() const {
        return year;
    }

    void setYear(const int value) {
        year = value;
    }

private:
    int year;
    const std::string type;
};

TEST_CASE("Example from README.md") {
    const std::string code = R"(
        import "mymodule" for MyFoo, Vec3

        class Main {
            static main() {
                var vec = Vec3.new(1.1, 2.2, 3.3)
                // Do something with "vec"

                var foo = MyFoo.new(2019, "Hello World")
                System.print("Foo type: %(foo.type) ")
                foo.year = 2020
                return foo
            }
        }
    )";

    // Create new VM
    wren::VM vm;

    // Create new module
    auto& m = vm.module("mymodule");

    // Declare new class
    auto& cls = m.klass<MyFoo>("MyFoo");

    // Optional constructor with optional name
    cls.ctor<int, const std::string&>("new");

    // Define variables
    cls.var<&MyFoo::message>("message");                // Direct access
    cls.prop<&MyFoo::getYear, &MyFoo::setYear>("year"); // As getter & getter
    cls.propReadonly<&MyFoo::getType>("type");          // Read only variable

    // Append some extra stuff to the "mymodule"
    m.append(R"(
        class Vec3 {
            construct new (x, y, z) {
                _x = x
                _y = y
                _z = z
            }
        }
    )");

    // Runs the code from the std::string as a "main" module
    vm.runFromSource("main", code);

    // Find the main() function
    // You can look for classes and their functions!
    auto mainClass = vm.find("main", "Main");
    auto main = mainClass.func("main()");

    auto res = main(); // Execute the function

    // Access the return value
    MyFoo* ptr = res.as<MyFoo*>();                     // As a raw pointer
    std::shared_ptr<MyFoo> sptr = res.shared<MyFoo>(); // As a shared ptr

    REQUIRE(ptr);
    REQUIRE(ptr == sptr.get());
}
