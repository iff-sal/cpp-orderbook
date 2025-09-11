cmake_minimum_required(VERSION 3.10)

# Set the project name
project(FinancialMatchingEngine)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(matching_engine main.cpp OrderBook.cpp)

set_target_properties(matching_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)