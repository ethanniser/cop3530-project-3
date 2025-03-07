cmake_minimum_required(VERSION 3.22)
project(Project3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include /opt/homebrew/include)
add_executable(Main main.cpp)
target_compile_options(Main PRIVATE -Wall -Werror)