# gs_libs
[![CircleCI](https://circleci.com/gh/ArnauPrat/gs_libs/tree/main.svg?style=svg)](https://circleci.com/gh/ArnauPrat/gs_libs/tree/main)

This repository contains single header C/C++ single libraries I use in my games.
For more details on how to use each library, look at the documentation section 
of each library source, and/or at the corresponding library test.

For now, libraries are tested using Clang-cl under Windows 10 using Visual Studio 2019, and Linux with clang 
for 64 bit architectures. Support for other platforms will be added soon.


| Library        | Description              | Supported platforms                |
|----------------|--------------------------|------------------------------------|
| gs_mem_alloc.h | Simple memory allocators | Windows/Clang-CL/64bits<br>Linux/Clang/64bits|
