cmake_minimum_required(VERSION 3.24)
project(NetworkTest)

set(CMAKE_CXX_STANDARD 17)

add_executable(NetworkTestServer main.cpp)
target_link_libraries(NetworkTestServer PUBLIC ws2_32)

add_executable(NetworkTestClient main_client.cpp)
target_link_libraries(NetworkTestClient PUBLIC ws2_32)

if(WIN32)
    target_compile_definitions(NetworkTestServer PUBLIC WINDOWS_PLATFORM)
    target_compile_definitions(NetworkTestClient PUBLIC WINDOWS_PLATFORM)
endif()