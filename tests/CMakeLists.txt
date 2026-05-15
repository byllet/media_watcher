cmake_minimum_required(VERSION 3.15)
project(tests)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare (
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME} 
                tests.cpp
              )

target_include_directories(${PROJECT_NAME} PRIVATE ../src)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})