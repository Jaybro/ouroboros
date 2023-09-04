# Ouroboros

[![build-and-test](https://github.com/Jaybro/ouroboros/workflows/build-and-test/badge.svg)](https://github.com/Jaybro/ouroboros/actions?query=workflow%3Abuild-and-test)

Ouroboros is a C++ header only library containing a cyclic deque, otherwise known as a [circular buffer](https://en.wikipedia.org/wiki/Circular_buffer), ring buffer, etc. The cyclic deque, named `ouroboros::cyclic_deque<>`, describes an object that provides a circular view of a contiguous sequence of objects with the first element of the sequence at position zero. A deque (double-ended queue) allows fast insertion and deletion at both its beginning and end.

The library is named after the [symbol](https://en.wikipedia.org/wiki/Ouroboros) depicting a serpent or dragon that eats its own tail and it represents the infinite amount of times a circular buffer has been implemented in code throughout history.

Available under the [MIT](https://en.wikipedia.org/wiki/MIT_License) license.

# Capabilities

* Interpret any contiguous sequence as a cyclic deque.
* Fast insertion and deletion at both its beginning and end.
* STL compliant. Provides the interface of a random access range.

# Examples

* [Minimal working example](./examples/cyclic_deque/cyclic_deque_minimal.cpp) for creating and using an `ouroboros::cyclic_deque<>`.

# Requirements

Minimum:

* A compiler that is compliant with the C++17 standard or higher.
* [CMake](https://cmake.org/). It is also possible to just copy and paste the [ouroboros](./src/) directory into an include directory.

Optional:

* [Doxygen](https://www.doxygen.nl). Needed for generating documentation.
* [Google Test](https://github.com/google/googletest). Used for running unit tests.

# Build

Build with [CMake](https://cmake.org/):

```console
$ mkdir build && cd build
$ cmake ../
$ cmake --build .
$ cmake --build . --target ouroboros_doc
$ cmake --install .
```

```cmake
find_package(Ouroboros REQUIRED)

add_executable(myexe main.cpp)
target_link_libraries(myexe PUBLIC Ouroboros::Ouroboros)
```
