---
title: 4. Supported types
---

# 4. Supported types

## 4.1. List of supported types and conversion

This is a list of supported types and how they are converted between C++ and Wren. Not everything can be mapped exactly (for example, integers are casted into doubles), so there are some compromises.

| C++ type | Wren type | Return value from `Any` |
| -------- | --------- | ------------------------ |
| signed char (int8_t) | number (64-bit float) | `.as<int8_t>()` |
| unsigned char (uint8_t) | number (64-bit float) | `.as<uint8_t>()` |
| signed short (int16_t) | number (64-bit float) | `.as<int16_t>()` |
| unsigned short (uint16_t) | number (64-bit float) | `.as<uint16_t>()` |
| signed int (int32_t) | number (64-bit float) | `.as<int32_t>()` |
| unsigned int (uint32_t) | number (64-bit float) | `.as<uint32_t>()` |
| signed long long (int64_t) | number (64-bit float) | `.as<int64_t>()` |
| unsigned long long (uint64_t) | number (64-bit float) | `.as<uint64_t>()` |
| float | number (64-bit float) | `.as<float>()` |
| double | number (64-bit float) | `.as<double>()`|
| char | number (64-bit float) | `.as<char>()` |
| char[N] | string | Not possible to get raw char array from Wren, use std::string |
| char* | string | Not possible to get raw char pointer from Wren, use std::string |
| std::string | string | `.as<std::string>()` |
| std::shared_ptr<T\> | T or null | `.as<T>()` or `.as<T*>()` or `.shared<T>()` |
| T (custom types) | foregin class of T | `.as<T>()` (if copy is supported) or `.as<T*>()` or `.as<T&>()` or `.shared<T>()` |

## 4.2. Unsupported types

| C++ type | Reason |
| -------- | ------ |
| std::unique_ptr<T\> | How would it work if you want to get `std::unique_ptr<T>&` ? Not possible to implement. |
| std::queue<T\> | Can't iterate over the elements without modifying the queue itself. |

## 4.3. STL Containers

The following std containers are supported and will be converted to a Wren type. Read the [11. STL containers]({{< ref "stl.md" >}}) section to understand more about STL containers and how to use them.

{{< hint warning >}}
**Warning**

By default the following STL containers below will be converted to a native type. If you add/bind any of these containers (except optional or variant) as a foregin class via `vm.module(...).klass<std::vector<int>>("VectorInt")` then that type will no longer be treated as a native type. Only the type you have registered, for example `std::vector<int>` will be treated as such, other types of the same container, for example `std::vector<float>` will still be treated as a native list. 

Whether the STD container is converted a Wren native list/map or into a Wren foreign class is checked at runtime.

Read the [11. STL containers]({{< ref "types.md" >}}) section to understand how it works.
{{< /hint >}}

| C++ type | Wren type | Return value from `Any` |
| -------- | --------- | ------------------------ |
| std::variant<A, B, C\> | A or B or C | `.as<A or B or C>()` or `.as<std::variant<A, B, C>>()` |
| std::optional<T\> | T or null | `.as<T>()` (only if not null) or `.as<std::optional<T>>()` |
| std::vector<T\> | native list of T | `.as<std::vector<T>>()` |
| std::list<T\> | native list of T | `.as<std::list<T>>()` |
| std::deque<T\> | native list of T | `.as<std::deque<T>>()` |
| std::set<T\> | native list of T | `.as<std::set<T>>()` |
| std::unordered_set<T\> | native list of T | `.as<std::unordered_set<T>>()` |
| std::map<std::string, T\> | native map of T | `.as<std::map<std::string, T>>()` |
| std::unordered_map <std::string, T\> | native map of T | `.as<std::unordered_map <std::string, T>>()` |
