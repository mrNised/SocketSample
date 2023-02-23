add_executable(TCPTestServer main.cpp)
target_link_libraries(TCPTestServer PUBLIC ws2_32)

add_executable(TCPTestClient main_client.cpp)
target_link_libraries(TCPTestClient PUBLIC ws2_32)

if(WIN32)
    target_compile_definitions(TCPTestServer PUBLIC WINDOWS_PLATFORM)
    target_compile_definitions(TCPTestClient PUBLIC WINDOWS_PLATFORM)
endif()