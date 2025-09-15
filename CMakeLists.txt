cmake_minimum_required(VERSION 3.10)

# Set the project name
project(FinancialMatchingEngine)

# GoogleTest Integration
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)


# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include external libs
include_directories(${CMAKE_SOURCE_DIR}/libs/FTXUI/include)



# Add the executable
add_executable(matching_engine main.cpp OrderBook.cpp)

enable_testing()

add_executable(
  run_tests
  tests/OrderBookTests.cpp
  OrderBook.cpp
)

target_link_libraries(matching_engine
    PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/FTXUI/build/libftxui-component.a
    ${CMAKE_SOURCE_DIR}/libs/FTXUI/build/libftxui-dom.a
    ${CMAKE_SOURCE_DIR}/libs/FTXUI/build/libftxui-screen.a
)

set_target_properties(matching_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(
  run_tests
  PRIVATE gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)