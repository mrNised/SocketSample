add_executable(UDPTestServer main.cpp)
target_link_libraries(UDPTestServer PUBLIC ws2_32)

add_executable(UDPTestClient main_client.cpp)
target_link_libraries(UDPTestClient PUBLIC ws2_32)

if(WIN32)
    target_compile_definitions(UDPTestServer PUBLIC WINDOWS_PLATFORM)
    target_compile_definitions(UDPTestClient PUBLIC WINDOWS_PLATFORM)
endif()