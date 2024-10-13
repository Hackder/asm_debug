Compile:
```bash
source ./envs
mkdir build
cd build
cmake ..
make
```

Compile example with plugin:
```bash
clang -Xclang -load -Xclang ./build/libasm_debug.dylib -Xclang -add-plugin -Xclang asm_debug testfile.c -target x86_64-apple-macos -fasm-blocks -g -O0
```
