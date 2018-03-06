cmake_minimum_required(VERSION 3.10.0)
project(ImplementacjePrzemyslowe)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB SRC
    "src/*.cpp"
)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-O2 -march=native -Wall -Wextra -pedantic")

add_executable(ImplementacjePrzemyslowe ${SRC})

target_link_libraries(ImplementacjePrzemyslowe ${OpenCV_LIBS})
target_include_directories(ImplementacjePrzemyslowe PRIVATE "./include/")
