cmake_minimum_required(VERSION 3.10)

project(base-streamer)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories("/usr/include/boost")

add_library(base64 SHARED
    inc/cpp-base64/base64.cpp)


add_executable(server_async server_async.cpp)
add_executable(camera_client camera_client.cpp)

target_link_libraries (server_async ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} base64)
target_link_libraries (camera_client ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} base64)


