cmake_minimum_required(VERSION 3.13)  
project(shm_hash_map)              
set(CMAKE_CXX_STANDARD 17)           

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=thread")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


include_directories(src)

file(GLOB UTILITY_SOUCE_FILES "src/utility/*.cpp")

set(SERVER_SOURCE_FILES src/server/main.cpp src/server/Server.cpp)

set(CLIENT_SOURCE_FILES src/client/Client.cpp)

add_executable(server ${SERVER_SOURCE_FILES} ${UTILITY_SOUCE_FILES})

add_executable(client ${CLIENT_SOURCE_FILES} ${UTILITY_SOUCE_FILES} src/client/main.cpp)

add_executable(perf_test_client ${CLIENT_SOURCE_FILES} ${UTILITY_SOUCE_FILES} src/client/perfTest.cpp)
