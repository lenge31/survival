Ref to CMakeLists.txt
The output file name MUST be out.test(used for autotest).

test framework　 
    googletest
    git clone https://github.com/google/googletest

build
    g++ test.cpp -lgtest -lpthread ../src/*.cpp -o out.test
