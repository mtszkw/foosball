cmake_minimum_required(VERSION 3.10.0)
project(ImplementacjePrzemyslowe)

function(addPrefixes listVar prefix out)
  set(ret)
  foreach(i ${listVar})
    list(APPEND ret ${prefix}${i})
  endforeach()
  set(${out} ${ret} PARENT_SCOPE)
endfunction()

function(addSuffixes listVar suffix out)
  set(ret)
  foreach(i ${listVar})
    list(APPEND ret ${i}${suffix})
  endforeach()
  set(${out} ${ret} PARENT_SCOPE)
endfunction()

function(toFastbuildArray listVar out)
  list(GET listVar 0 elem)
  set(ret "{\"${elem}\"")
  list(LENGTH L1 size)
  math(EXPR size "${size}-1")
  if(${size} GREATER 0)
    foreach(i RANGE 1 ${size})
      LIST(GET listVar ${i} elem)
      message(${elem})
      set(ret "${ret}, \"${elem}\"")
    endforeach()
  endif()
  set(ret "${ret}}")
  set(${out} ${ret} PARENT_SCOPE)
endfunction()


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB SRC
    "src/*.cpp"
)

set (CMAKE_CXX_STANDARD 17)

add_executable(ImplementacjePrzemyslowe ${SRC})
target_link_libraries(ImplementacjePrzemyslowe ${OpenCV_LIBS})
target_include_directories(ImplementacjePrzemyslowe PRIVATE "./include/")

set(SOURCE_TEST_FILES test/TestCase.cpp)

add_executable (Test ${SOURCE_TEST_FILES})

target_link_libraries(Test ${OpenCV_LIBS})
target_include_directories(Test PRIVATE "./include/")

enable_testing()

add_test (NAME Test1 COMMAND Test
    --report_level=detailed
    --log_level=all
    --color_output=yes
)

set(IncludeDirs ${OpenCV_INCLUDE_DIRS})
list(APPEND IncludeDirs "./include/")
addPrefixes("${IncludeDirs}" " -I" IncludeDirs)
string(REPLACE ";" "" IncludeDirs "${IncludeDirs}")
addPrefixes("${OpenCV_LIBS}" " -l" IncludeLibs)
addSuffixes("${IncludeLibs}" "341" IncludeLibs)
string(REPLACE ";" "" IncludeLibs "${IncludeLibs}")
set(libsPath " -L${OpenCV_INSTALL_PATH}/lib")

file(WRITE fbuild.bff ".Compiler = '${CMAKE_CXX_COMPILER}' \n")

file(APPEND fbuild.bff ".CompilerOptions = '\"%1\"' 
+ ' -c' 
+ ' -o\"%2\"'
+ \"${IncludeDirs}${IncludeLibs}${libsPath}\"\n
+ ' -lstdc++fs'\n
")

file(APPEND fbuild.bff ".Linker = '${CMAKE_CXX_COMPILER}' \n")

file(APPEND fbuild.bff ".LinkerOptions = ' \"%1\"' 
+ ' -o\"%2\"' 
+ \"${IncludeDirs}${IncludeLibs}${libsPath}\"\n
+ ' -lstdc++fs'\n
")

toFastbuildArray("${SRC}" fba)

file(APPEND fbuild.bff 
"ObjectList( '${PROJECT_NAME}-Lib' ) {
  .CompilerInputFiles = ${fba}
  .CompilerOutputPath = '/out/' 
} 
Executable( '${PROJECT_NAME}' ) { 
  .Libraries = { \"${PROJECT_NAME}-Lib\" } 
  .LinkerOutput = '/out/${PROJECT_NAME}.exe' 
} // All //------------------------------------------------------------------------------
Alias( 'all' ) { 
  .Targets = { '${PROJECT_NAME}' }
}\n"
)

