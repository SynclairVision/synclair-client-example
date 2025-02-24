cmake_minimum_required(VERSION 3.10)

# Projektets namn
project(Automation)

# Inställningar för C++-standarden
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Inkludera src-mappen
include_directories(src)

# Hitta GoogleTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Inkludera cpprestsdk-sökvägen manuellt
set(CPPRESTSDK_INCLUDE_DIRS "/usr/include/cpprest")
set(CPPRESTSDK_LIBS "/usr/lib/x86_64-linux-gnu/libcpprest.so")

# Lägg till exekverbar fil för main-programmet
add_executable(main_program src/main.cpp)

# Lägg till exekverbar fil för tester
add_executable(test_program tests/test.cpp)
target_link_libraries(test_program ${GTEST_LIBRARIES} pthread)

# Lägg till exekverbar fil för benchmarks
add_executable(benchmark_program benchmarks/benchmark.cpp)
target_link_libraries(benchmark_program benchmark pthread)

# Lägg till exekverbar fil för systemtest
add_executable(system_test_program tests/system_test.cpp)
target_include_directories(system_test_program PRIVATE ${CPPRESTSDK_INCLUDE_DIRS})
target_link_libraries(system_test_program ${GTEST_LIBRARIES} ${CPPRESTSDK_LIBS} pthread ssl crypto)
