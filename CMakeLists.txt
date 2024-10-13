cmake_minimum_required(VERSION 3.13)

project(MyClangPlugin LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "arm64")

find_package(Clang REQUIRED CONFIG)
include_directories(${CLANG_INCLUDE_DIRS})

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_library(asm_debug SHARED src/main.cpp)
set_target_properties(asm_debug PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
    LINK_FLAGS "-shared"
)

target_link_libraries(asm_debug PRIVATE LLVM clang)
target_link_libraries(asm_debug PRIVATE LLVM clangSupport clangFrontend clangAST clangBasic clangRewrite)

