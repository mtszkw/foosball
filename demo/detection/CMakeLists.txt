cmake_minimum_required(VERSION 3.10.0)
project(aruco_demo)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCES
    detection.cpp
)

set (CMAKE_CXX_STANDARD 17)

add_executable(detection_demo ${SOURCES})

target_link_libraries(detection_demo ${OpenCV_LIBS})
target_include_directories(detection_demo PRIVATE "../../include/")
