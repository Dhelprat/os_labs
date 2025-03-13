cmake_minimum_required(VERSION 3.10)
project(distributed_computing)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ручное указание путей к ZeroMQ
set(ZMQ_PREFIX "/opt/homebrew/opt/zeromq")

# Подключение заголовков ZeroMQ
include_directories(${ZMQ_PREFIX}/include)

# Создание общей библиотеки
add_library(common_lib
        src/parent.cpp
        src/message.h
        src/nodes.h
)

# Создание исполняемых файлов
add_executable(control src/control.cpp)
add_executable(computing src/child.cpp)

# Связывание с ZeroMQ
target_link_libraries(control
        PRIVATE
        common_lib
        ${ZMQ_PREFIX}/lib/libzmq.dylib
)

target_link_libraries(computing
        PRIVATE
        common_lib
        ${ZMQ_PREFIX}/lib/libzmq.dylib
)

# Опции компилятора
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(control PRIVATE -Wall -Wextra)
    target_compile_options(computing PRIVATE -Wall -Wextra)
endif()