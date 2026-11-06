cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)

# 1. Define the project name
project(trade-settlement LANGUAGES CXX)

# 2. Main Application Target (Renamed from 'build' to 'trade_app' for clarity)
add_executable(trade_app main.cpp)

# (Optional TSan) Apply ThreadSanitizer directly to the application target
target_compile_options(trade_app PRIVATE -fsanitize=thread -g -fno-omit-frame-pointer)
target_link_options(trade_app PRIVATE -fsanitize=thread)

# 3. Testing Target Configuration
# Note: CMake has a built-in standard module 'CTest' that defines BUILD_TESTING for you
include(CTest) 

if(BUILD_TESTING)
    # Bring in GoogleTest
    add_subdirectory(external/googletest)
    include(GoogleTest)
    
    # Create the test executable (pointed to your test file, e.g., test_main.cpp)
    add_executable(test_app test.cpp) 
    
    # Link GoogleTest to the test target (NOT the project name)
    target_link_libraries(test_app PRIVATE gtest_main)
    
    # (Optional TSan) If you want your tests to run with ThreadSanitizer too:
    # target_compile_options(test_app PRIVATE -fsanitize=thread -g -fno-omit-frame-pointer)
    # target_link_options(test_app PRIVATE -fsanitize=thread)

    # Discover tests from the test target (NOT the project name)
    gtest_discover_tests(test_app)
endif()
