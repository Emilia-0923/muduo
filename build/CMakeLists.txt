cmake_minimum_required(VERSION 3.10)

project(Server)

# 设置默认的构建类型为 Release，如果用户没有指定的话
if(NOT TYPE)
    set(TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# 设置编译器标志
if(TYPE STREQUAL "Debug")
    # 在 Debug 模式下添加 -g 标志
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

add_executable(${PROJECT_NAME} ../source/Main.cpp)