cmake_minimum_required(VERSION 3.10.0)
project(calibration_demo)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCES
    ../../src/cameraCalibration.cpp
    main.cpp
)

set (CMAKE_CXX_STANDARD 17)

add_executable(calibration_demo ${SOURCES})

target_link_libraries(calibration_demo ${OpenCV_LIBS})
target_include_directories(calibration_demo PRIVATE "../../include/")

if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(calibration_demo stdc++fs)
endif()