cmake_minimum_required(VERSION 3.20)
project(lua_bz3 C)

set(CMAKE_C_STANDARD 11)
set(LUA_INCLUDE E:\\pyproject\\Lua_5.4_WinBuild\\lua-5.4.7\\src) # D:\\conda\\envs\\py310\\Library\\include
set(LUA_LIB E:\\pyproject\\Lua_5.4_WinBuild\\cmake-build-release-visual-studio)

link_directories(${LUA_LIB})
include_directories(${LUA_INCLUDE} ./dep/include)

add_compile_definitions(VERSION="1.5.1.r2-g61c764f")

add_library(bz3 SHARED src/lbz3.c dep/src/libbz3.c)
target_link_libraries(bz3 lua)
