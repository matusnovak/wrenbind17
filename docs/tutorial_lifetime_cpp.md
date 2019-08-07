# Tutorial: Lifetime of objects passed into Wren 

In the previous section we have discussed what happens with objects that are returned from Wren, but what about the other way around?

It's very simple in this case. There are only 4 different scenarios:

* Pass class `Foo` as a value. The Wren will create a copy of it. 
* Pass class `Foo*` as a pointer. The instance will be wrapped into `std::shared_ptr<Foo>` **but won't free it**. So it will be alive inside of Wren until you free the instance on C++ side. Wren's garbage collector won't free this instance.
* Pass class `Foo&` as a reference. This is the same case as passing a pointer.
* Pass class `std::shared_ptr<Foo>` as a shared pointer. The lifetime of this object will be extended by Wren. So both C++ and Wren will manage the lifetime of this. If you lose the shared pointer on C++ side, and Wren holds the instance, then it will be freed by Wren's garbage collector only when the instance inside of Wren gets no longer used/referenced.
