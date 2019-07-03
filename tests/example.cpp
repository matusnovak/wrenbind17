#include <catch2/catch.hpp>
#include <wrenbind17/wrenbind17.hpp>

namespace wren = wrenbind17;

class MyFoo {
public:
    MyFoo(const int year, const std::string& message)
        : year(year),
          message(message),
          type("MyFoo") {
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

    wren::VM vm;                               // Create new VM

    auto& m = vm.module("mymodule");           // Create new module

    auto& cls = m.klass<MyFoo>("MyFoo");       // Add foreign class to the module
    cls.ctor<int, const std::string&>("new");  // Optional constructor with optional name
    cls.var<&MyFoo::message>("message");       // Define variable by direct access
    cls.prop<&MyFoo::getYear, &MyFoo::setYear>("year"); // Define variable as getter & getter
    cls.propReadonly<&MyFoo::getType>("type"); // Read only variable

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

    vm.runFromSource("main", code); // Runs the code from the std::string as a "main" module
    auto main = vm.find("main", "Main").func("main()"); // Find the main() function

    auto res = main(); // Execute the function
    MyFoo* ptr = res.as<MyFoo*>(); // Access the return value
    std::shared_ptr<MyFoo> sptr = res.shared<MyFoo>(); // Access the return value as shared ptr
    assert(ptr);
    assert(ptr == sptr.get());
}
