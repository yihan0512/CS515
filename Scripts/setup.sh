cd /LLVM_ROOT/build
cmake /LLVM_ROOT/llvm
cd /LLVM_ROOT/build/lib/Transforms/CS515_Project
make

cd /tests/HelloWorld
clang -O0 -S -emit-llvm HelloWorld.cpp
cd /tests/conditionalSum
clang -O0 -S -emit-llvm main.cpp 
# cd /tests/example
# clang -O0 -S -emit-llvm main.cpp
