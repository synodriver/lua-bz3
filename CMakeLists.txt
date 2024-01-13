cmake_minimum_required(VERSION 3.20)
project(lua_bz3 C)

set(CMAKE_C_STANDARD 11)

link_directories("D:\\conda\\envs\\py310\\Library\\lib" D:\\conda\\envs\\py310\\Library\\bin)
include_directories(D:\\conda\\envs\\py310\\Library\\include ./dep/include)

add_compile_definitions(VERSION="1.4.0.r3-gd9e5e15")

add_library(bz3 SHARED src/lbz3.c dep/src/libbz3.c)
target_link_libraries(bz3 lua)
