cmake_minimum_required(VERSION 3.22)
project(Project3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

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

add_executable(Main 
    src/main.cpp
    src/lib.h
    src/lib.cpp
)

#For some reason had to be added to make main run
if (WIN32)
    target_link_libraries(Main PRIVATE ws2_32)
    target_link_libraries(Main PRIVATE ws2_32 mswsock)
endif()

target_include_directories(Main PRIVATE
    src
    ${asio_SOURCE_DIR}/asio/include
    ${crow_SOURCE_DIR}/include
)

target_compile_options(Main PRIVATE -Wall -Werror)

# Tests

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2 
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)


add_executable(Tests
    test/test.cpp
    src/lib.h
    src/lib.cpp
)
target_include_directories(Tests PRIVATE
    src
)
target_compile_options(Tests PRIVATE -Wall -Werror)
target_link_libraries(Tests PRIVATE Catch2::Catch2WithMain)