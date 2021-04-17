## Ryan Green's Compiler project for EECS 5183 Compiler Theory

The project is written in C++ with C++17 language features. You will need LLVM installed to build it.

Build the project using the following command:

``g++ `llvm-config --cxxflags --ldflags --system-libs --libs core` -std=c++17 -o Compiler -I Include -g $(find Source -type f -iregex ".*\.cpp")``

Then run the resulting output using:

`./Compiler`