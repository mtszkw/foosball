cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE )
project(ImplementacjePrzemyslowe)

include(cmake/clang-cxx-dev-tools.cmake)

# Setting variables
file(GLOB_RECURSE HEADERS "include/*.hpp")
file(GLOB_RECURSE SRC "src/*.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CONFIGURATION_FILE "configuration.json")

# Copy files
configure_file(${CONFIGURATION_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${CONFIGURATION_FILE} COPYONLY)

# Add OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Add executable
add_executable(${PROJECT_NAME} ${CONFIGURATION_FILE} ${HEADERS} ${SRC})
set_source_files_properties(${HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${CONFIGURATION_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE "./include/")

# _____________________________________________________________________________

# Unit tests source files
set(SOURCE_TEST_FILES 
    test/TestCase.cpp
    test/TestAruco.cpp

    src/aruco/aruco.cpp
    )

add_executable (${PROJECT_NAME}_tests ${SOURCE_TEST_FILES})
target_link_libraries(${PROJECT_NAME}_tests ${OpenCV_LIBS})
target_include_directories(${PROJECT_NAME}_tests PRIVATE "./include/")
enable_testing()
add_test (NAME Test1 COMMAND Test)

if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
    target_link_libraries(${PROJECT_NAME}_tests stdc++fs)
endif()

include(fastbuild.cmake)
generate_fastbuild(${PROJECT_NAME};${PROJECT_NAME}_tests)
