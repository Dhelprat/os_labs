cmake_minimum_required(VERSION 3.27)
project(os_labs)

set(CMAKE_CXX_STANDARD 14)

add_executable(os_labs
        lab1/src/child2.c
        lab1/src/child1.c
        lab1/src/parent.c)
