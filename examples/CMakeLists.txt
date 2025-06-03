cmake_minimum_required(VERSION 3.16)


set(CMAKE_BUILD_TYPE Debug)

add_executable(napys_example main.c)

target_link_libraries(napys_example PRIVATE SDL3::SDL3 Napys)