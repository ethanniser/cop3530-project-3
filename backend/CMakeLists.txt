cmake_minimum_required(VERSION 3.22)
project(Project3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-28-0  
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(asio)

set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include)

FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG        v1.2.1  
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(crow)

add_executable(Main main.cpp)

target_include_directories(Main PRIVATE
    ${asio_SOURCE_DIR}/asio/include
    ${crow_SOURCE_DIR}/include
)

target_compile_options(Main PRIVATE -Wall -Werror)