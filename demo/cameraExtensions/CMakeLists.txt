cmake_minimum_required(VERSION 3.10.0)
project(camera_extensions_demo)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCES
    ../../src/cameraExtensions/cameraExtensions.cpp
    main.cpp
)

set (CMAKE_CXX_STANDARD 17)

add_executable(camera_extensions_demo ${SOURCES})

target_link_libraries(camera_extensions_demo ${OpenCV_LIBS})
target_include_directories(camera_extensions_demo PRIVATE "../../include/")

if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(camera_extensions_demo stdc++fs)
endif()