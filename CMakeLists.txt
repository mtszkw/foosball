cmake_minimum_required(VERSION 3.10.0)
project(ImplementacjePrzemyslowe)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB SRC
    "src/*.cpp"
)

set (CMAKE_CXX_STANDARD 17)

add_executable(ImplementacjePrzemyslowe ${SRC})

target_link_libraries(ImplementacjePrzemyslowe ${OpenCV_LIBS})
target_include_directories(ImplementacjePrzemyslowe PRIVATE "./include/")

set(SOURCE_TEST_FILES 
    test/TestCase.cpp
    test/TestAruco.cpp

    src/aruco.cpp
    )

add_executable (Test ${SOURCE_TEST_FILES})

target_link_libraries(Test ${OpenCV_LIBS})
target_include_directories(Test PRIVATE "./include/")

enable_testing()

add_test (NAME Test1 COMMAND Test)

if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(ImplementacjePrzemyslowe stdc++fs)
    target_link_libraries(Test stdc++fs)
endif()
