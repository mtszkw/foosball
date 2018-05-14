set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(addPrefixes listVar prefix out)
  set(ret)
  foreach(i ${listVar})
    list(APPEND ret ${prefix}${i})
  endforeach()
  set(${out} ${ret} PARENT_SCOPE)
endfunction()

function(get_libraries out target)
  get_target_property(libs ${TARGET} LINK_LIBRARIES)
  set(ret)
  foreach(lib ${libs})
    if(TARGET ${lib})
      get_target_property(path ${lib} LOCATION)
      list(APPEND ret " ${path}")
    else()
      list(APPEND ret " -l${lib}")
    endif()
  endforeach()
  set(${out} ${ret} PARENT_SCOPE)
endfunction()

function(toFastbuildArray listVar out)
  list(GET listVar 0 elem)
  set(ret "{\"${elem}\"")
  list(LENGTH listVar size)
  #DBG(size)
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

function(DBG VAR)
  message("${VAR}: ${${VAR}}")
endfunction()

function(add_source_file out SOURCE)
  get_target_property(HEADER_FILE_ONLY ${SOURCE} HEADER_FILE_ONLY)
  if(NOT HEADER_FILE_ONLY)
    
    get_source_file_property(LOCATION ${SOURCE} LOCATION)
    get_source_file_property(COMPILE_DEFINITIONS ${SOURCE} COMPILE_DEFINITIONS)
    get_source_file_property(COMPILE_FLAGS ${SOURCE} COMPILE_FLAGS)
    get_source_file_property(COMPILE_OPTIONS ${SOURCE} COMPILE_OPTIONS)
    get_source_file_property(INCLUDE_DIRECTORIES ${SOURCE} INCLUDE_DIRECTORIES)
    
    
  endif()
endfunction()


macro(get_gcc_compile_flags target flags)
  string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" name)
  set(flags "${${name}} ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_COMPILER_ARG1}")
  get_target_property(value ${target} COMPILE_FLAGS)
  if (value)
    list(APPEND flags ${value})
  endif()
  get_target_property(value ${target} COMPILE_DEFINITIONS)
  if (value)
    foreach(item ${value})
      list(APPEND flags "-D${item}")
    endforeach()
  endif()
  get_directory_property(value COMPILE_DEFINITIONS)
  if (value)
    list(APPEND flags ${value})
  endif()
  get_directory_property(value INCLUDE_DIRECTORIES)
  if (value)
    foreach(item ${value})
      list(APPEND flags "-I${item}")
   endforeach()
  endif()
  get_target_property(value ${target} INCLUDE_DIRECTORIES)
  if (value)
    foreach(item ${value})
      list(APPEND flags "-I${item}")
   endforeach()
  endif()
  get_target_property(value ${target} COMPILE_OPTIONS)
  if (value)
    foreach(item ${value})
      list(APPEND flags "${item}")
   endforeach()
  endif()
  #separate_arguments(flags)
endmacro()

function(add_fastbuild_target out TARGET)
  get_target_property(TYPE ${TARGET} TYPE)
  DBG(TYPE)
  get_target_property(LINK_FLAGS  ${TARGET} LINK_FLAGS)
  #DBG(LINK_FLAGS)
  get_target_property(OUTPUT_NAME   ${TARGET} OUTPUT_NAME )
  #DBG(OUTPUT_NAME)
  get_target_property(LIBRARY_OUTPUT_DIRECTORY ${TARGET} LIBRARY_OUTPUT_DIRECTORY )
  #DBG(LIBRARY_OUTPUT_DIRECTORY)
  get_target_property(NAME ${TARGET} NAME )
  #DBG(NAME)
  get_target_property(SOURCES ${TARGET} SOURCES)
  #DBG(SOURCES)
  get_target_property(COMPILE_FLAGS ${TARGET} COMPILE_FLAGS)
  DBG(COMPILE_FLAGS)
  
  get_gcc_compile_flags(${TARGET} flags)
  #DBG(flags)
  
  
  list(GET SOURCES 0 elem)
  DBG(elem)
  get_source_file_property(RESULT ${elem} COMPILE_FLAGS)
  DBG(RESULT)
  
  get_libraries(IncludeLibs target)
  #DBG(IncludeLibs)
  string(REPLACE ";" " " IncludeLibs "${IncludeLibs}")
  string(REPLACE ";" " " flags "${flags}")
  toFastbuildArray("${SOURCES}" fba)
  


  set(ret "
ObjectList( '${NAME}-Lib' ) {
  .CompilerInputFiles = ${fba}
  .CompilerOutputPath = '/out/' 
  .CompilerOptions = '%1' 
  + ' -c' 
  + ' -o \"%2\" '
  + '${flags}'
}")
  if(TYPE STREQUAL "EXECUTABLE")
  set(ret "${ret} 
Executable( '${NAME}' ) {
  .Libraries = { '${NAME}-Lib' } 
  .LinkerOutput = '$<TARGET_FILE:${TARGET}>'
  .LinkerOptions = '%1' 
  + ' -o \"%2\"' 
  + '${IncludeLibs}'
}
"
  )
  endif()
  set(${out} "${${out}}${ret}" PARENT_SCOPE)
endfunction()

function(init_fastbuild out)
  set(ret ".Compiler = '${CMAKE_CXX_COMPILER}'\n.Linker = '${CMAKE_CXX_COMPILER}'\n")
  set(${out} "${ret}" PARENT_SCOPE)
endfunction()

function(alias_all_fastbuild out TARGETS)
  #DBG(TARGETS)
  set(ret)
  foreach(i ${TARGETS})
    get_target_property(NAME ${i} NAME )
    list(APPEND ret ${NAME})
  endforeach()
  #DBG(ret)
  toFastbuildArray("${ret}" ret)
  set(ret "\nAlias( 'all' ) {\n  .Targets = ${ret}\n}")
  set(${out} "${${out}}${ret}" PARENT_SCOPE)
endfunction()

function(generate_fastbuild TARGETS)
  set(fastbuildfile "")
  init_fastbuild(fastbuildfile)
  foreach(i ${TARGETS})
    add_fastbuild_target(fastbuildfile ${i})
  endforeach()
  alias_all_fastbuild(fastbuildfile "${TARGETS}")
  file(GENERATE OUTPUT fbuild.bff CONTENT ${fastbuildfile})
endfunction()
