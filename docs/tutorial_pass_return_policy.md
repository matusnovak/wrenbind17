# Tutorial: Pass and return C++ variables policy

WrenBind17 supports passing C++ variables into Wren (either by calling a Wren function or calling C++ function from Wren) by a copy, pointer, shared pointer, or a reference.

## Passing C++ variables to Wren

This applies when calling a Wren function. For example, you want to pass some custom class into a Wren function.

* Pass by a copy - When passing by a copy the Wren will create a copy of the class instance.
* pass by a pointer - The Wren will only keep the pointer, no copies are created.
* Pass by a move - The move will be handled and no copies will be created.
* Pass by a reference - Same as a copy.
* Pass by a constant reference - Same as a copy.
* Pass by a shared pointer - C++ and Wren will share the same smart pointer, no copies are created.

## Returning C++ variables to Wren

This applies when calling a C++ function (a member function of a class) that returns a custom class. For example, you have a factory or some global resource manager and you want to access some C++ class instances inside of Wren.

* Return by a copy - A copy will be created.
* Return by a move - The instance will be moved into a shared pointer wrapper used by Wren, no copies will be created.
* Return by a pointer - No copy will be created. The pointer will be moved into a shared_ptr without a delete function.
* Return by a reference - Same as pointer.
* Return by a const reference - Same as pointer.
* Return by a shared pointer - C++ and Wren will share the same smart pointer, no copies are created.
