set(LOG "")
set(configTypes "DEBUG;RELEASE;RELWITHDEBINFO;MINSIZEREL")
set(anyConfigTypes ";_DEBUG;_RELEASE;_RELWITHDEBINFO;_MINSIZEREL")

macro(dbgf variable)
  set(LOG "${LOG}${variable}: ${${variable}}\n")
endmacro()

function(get_libraries out target)
  get_target_property(libs ${TARGET} LINK_LIBRARIES)
  set(ret)
  foreach(lib ${libs})
    if(TARGET ${lib})
      foreach(configType ${anyConfigTypes})
        set(name "IMPORTED_IMPLIB${configType}")
        get_target_property(path ${lib} ${name})
        if(path)
          break()
        endif()
      endforeach()
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
  math(EXPR size "${size}-1")
  if(${size} GREATER 0)
    foreach(i RANGE 1 ${size})
      LIST(GET listVar ${i} elem)
      #message(${elem})
      set(ret "${ret}, \"${elem}\"")
    endforeach()
  endif()
  set(ret "${ret}}")
  set(${out} ${ret} PARENT_SCOPE)
endfunction()

function(DBG VAR)
  message("${VAR}: ${${VAR}}")
endfunction()

macro(get_compile_flags target flags)
  set(flags "")
  foreach(configType ${configTypes})
    string(TOUPPER "CMAKE_CXX_FLAGS_${configType}" name)
    set(flags "${flags}$<$<CONFIG:${configType}>:${name}> ")
  endforeach()
  dbgf(flags)
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
  get_target_property(value ${target} CMAKE_CXX_STANDARD)
  if (value AND CMAKE_COMPILER_IS_GNUCC)
    list(APPEND flags "-std=c++${item}")
  elseif(${CMAKE_CXX_STANDARD} AND CMAKE_COMPILER_IS_GNUCC)
    #message("appending")
    list(APPEND flags "-std=c++${CMAKE_CXX_STANDARD}")
  endif()
endmacro()

function(add_source_file SOURCE out name)
  get_source_file_property(HEADER_FILE_ONLY ${SOURCE} HEADER_FILE_ONLY)
  if(NOT HEADER_FILE_ONLY)
    set(srcflags "")
    get_source_file_property(LOCATION ${SOURCE} LOCATION)
    set(objectName ${LOCATION})
    string(REPLACE "/" "-" objectName "${objectName}")
    string(REPLACE "\\" "-" objectName "${objectName}")
    string(REPLACE "." "-" objectName "${objectName}")
    string(REPLACE ":" "-" objectName "${objectName}")
    set(${name} ${objectName} PARENT_SCOPE)
    get_source_file_property(value ${SOURCE} COMPILE_DEFINITIONS)
    if (value)
      foreach(item ${value})
        list(APPEND srcflags "-D${item}")
      endforeach()
    endif()
    get_source_file_property(value ${SOURCE} COMPILE_FLAGS)
    if (value)
      foreach(item ${value})
        list(APPEND srcflags "${item}")
      endforeach()
    endif()
    get_source_file_property(value ${SOURCE} COMPILE_OPTIONS)
    if (value)
      foreach(item ${value})
        list(APPEND srcflags "${item}")
      endforeach()
    endif()
    get_source_file_property(value ${SOURCE} INCLUDE_DIRECTORIES)
    if (value)
      foreach(item ${value})
        list(APPEND srcflags "-I${item}")
      endforeach()
    endif()
    string(REPLACE ";" " " srcflags "${srcflags}")
    set(outputOpt "/Fo")
    if(CMAKE_COMPILER_IS_GNUCC)
      set(outputOpt "-o")
    endif()
    set(ret2 "
ObjectList( '${objectName}' ) {
  .CompilerInputFiles = {'${LOCATION}'}
  .CompilerOutputPath = '/out/' 
  .CompilerOptions + ' %1 -c ${outputOpt}\"%2\" ${srcflags}'
}")
    set(${out} "${${out}}${ret2}" PARENT_SCOPE)
  endif()
endfunction()

macro(map_type wtf out)
  dbg(wtf)
  if(${type} STREQUAL "STATIC_LIBRARY")
    set(out "STATIC")
  endif()
  if(${type} STREQUAL "MODULE_LIBRARY")
    set(out "MODULE")
  endif()
  if(${type} STREQUAL "SHARED_LIBRARY")
    set(out "SHARED")
  endif()
  if(${type} STREQUAL "INTERFACE_LIBRARY")
    set(out "what")
  endif()
  if(${type} STREQUAL "EXECUTABLE")
    set(out "EXE")
  endif()
endmacro()

function(add_fastbuild_target out TARGET)
  get_target_property(TYPE ${TARGET} TYPE)
  get_target_property(LINK_FLAGS ${TARGET} LINK_FLAGS)
  if(NOT LINK_FLAGS)
    set(LINK_FLAGS "")
  endif()
  get_target_property(NAME ${TARGET} NAME )
  get_target_property(SOURCES ${TARGET} SOURCES)
  get_compile_flags(${TARGET} flags)
  if(TYPE STREQUAL "SHARED_LIBRARY")
    list(APPEND flags "-shared")
  endif()
  get_libraries(IncludeLibs target)
  string(REPLACE ";" " " IncludeLibs "${IncludeLibs}")
  string(REPLACE ";" " " flags "${flags}")
  set(ret "\n.CompilerOptions = '${flags}'\n")
  set(libraries "")
  foreach(i ${SOURCES})
    add_source_file(${i} ret srcName)
    list(APPEND libraries "${srcName}")
  endforeach()
  toFastbuildArray("${libraries}" libraries)
  if(TYPE STREQUAL "EXECUTABLE" OR TYPE STREQUAL "SHARED_LIBRARY")
  set(outputOpt "/OUT:")
  if(CMAKE_COMPILER_IS_GNUCC)
    set(outputOpt "-o")
  endif()
  
  if(${TYPE} STREQUAL "STATIC_LIBRARY")
    set(mappedType "STATIC")
  endif()
  if(${TYPE} STREQUAL "MODULE_LIBRARY")
    set(mappedType "MODULE")
  endif()
  if(${TYPE} STREQUAL "SHARED_LIBRARY")
    set(mappedType "SHARED")
  endif()
  if(${TYPE} STREQUAL "INTERFACE_LIBRARY")
    set(mappedType "what")
  endif()
  if(${TYPE} STREQUAL "EXECUTABLE")
    set(mappedType "EXE")
  endif()
  
  
  dbg(mappedType)
  set(CMAKE_LINKER_FLAGS "CMAKE_${mappedType}_LINKER_FLAGS")
  dbg(${CMAKE_LINKER_FLAGS})
  set(LINK_FLAGS "${LINK_FLAGS} ${${CMAKE_LINKER_FLAGS}} ")
  foreach(configType ${configTypes})
    set(name "${CMAKE_LINKER_FLAGS}_${configType}")
    dbg(${name})
    set(LINK_FLAGS "${LINK_FLAGS}$<$<CONFIG:${configType}>:${${name}}>")
  endforeach()
  set(LINK_FLAGS "${LINK_FLAGS} ")
  foreach(configType ${configTypes})
    set(name "LINK_FLAGS_${configType}")
    get_target_property(${name} ${TARGET} ${name})
    #dbg(${name})
    if(${name})
      set(LINK_FLAGS "${LINK_FLAGS}$<$<CONFIG:${configType}>:${${name}}>")
    endif()
  endforeach()
  
  
  set(ret "${ret} 
Executable( '${NAME}' ) {
  .Libraries = ${libraries} 
  .LinkerOutput = '$<TARGET_FILE:${TARGET}>'
  .LinkerOptions = '%1' 
  + ' ${outputOpt}\"%2\"' 
  + '${IncludeLibs}'
  + '${LINK_FLAGS}'
}
"
  )
  endif()
  set(${out} "${${out}}${ret}" PARENT_SCOPE)
endfunction()

function(init_fastbuild out)
  set(linker ${CMAKE_LINKER})
  if(CMAKE_COMPILER_IS_GNUCC)
    set(linker ${CMAKE_CXX_COMPILER})
  endif()
  set(ret ".Compiler = '${CMAKE_CXX_COMPILER}'\n.Linker = '${linker}'\n")
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
  file(GENERATE OUTPUT "fbuild$<CONFIG>.bff" CONTENT ${fastbuildfile})
  file(GENERATE OUTPUT "log$<CONFIG>.txt" CONTENT "${LOG}")
endfunction()