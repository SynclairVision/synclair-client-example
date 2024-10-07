cmake_minimum_required(VERSION 3.12)
project(synclair-client-example)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directory for message-definitions
include_directories(message-definitions)

# Add your client.cpp file
add_executable(synclair-client-example client.cpp)

# Link with the message-definitions library
target_include_directories(synclair-client-example PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/message-definitions
)

if(WIN32)
  target_link_libraries(synclair-client-example wsock32 ws2_32)
endif()

# Install the executable
install(TARGETS synclair-client-example DESTINATION bin)