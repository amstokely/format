cmake_minimum_required(VERSION 4.0)
project(format LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 23)

include_directories(external)
add_subdirectory(src)
add_subdirectory(test)
add_executable(main main.cpp)
target_link_libraries(main PRIVATE format)