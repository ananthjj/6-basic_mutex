cmake_minimum_required(VERSION 2.8)
project('assignment-6')

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall")
add_executable(main main.c simple_mutex.c)
TARGET_LINK_LIBRARIES(main pthread)
