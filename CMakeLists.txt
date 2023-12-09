cmake_minimum_required(VERSION 3.23.0)
project(main VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_executable(client client.cpp)
add_executable(server server.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(client PRIVATE fmt::fmt)
target_link_libraries(server PRIVATE fmt::fmt)

