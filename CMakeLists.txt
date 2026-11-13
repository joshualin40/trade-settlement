cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)

project(trade-settlement LANGUAGES CXX)

option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

add_subdirectory(external/googletest)

add_executable(trade_settlement 
                main.cpp)

target_link_libraries(trade_settlement gtest_main)
include(GoogleTest)
gtest_discover_tests(trade_settlement)
enable_testing()

if(ENABLE_TSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(trade_settlement PRIVATE -fsanitize=thread -g -O1)
        target_link_options(trade_settlement PRIVATE -fsanitize=thread)
    else()
        message(WARNING "ThreadSanitizer is only supported with GCC or Clang.")
    endif()
endif()
