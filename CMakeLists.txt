cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE )
project(ImplementacjePrzemyslowe)

include(cmake/clang-cxx-dev-tools.cmake)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set (CMAKE_CXX_STANDARD 17)

file(GLOB SRC
    "src/*.cpp"
)

set(HEADER_FILES
include/aruco
include/detection
include/cameraCalibration
include/table
)

add_executable(ImplementacjePrzemyslowe ${SRC} ${HEADER_FILES})
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

include(fastbuild.cmake)
generate_fastbuild("ImplementacjePrzemyslowe;Test")
