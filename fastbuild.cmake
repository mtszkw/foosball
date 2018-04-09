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

function(DBG VAR)
  message("${VAR}: ${${VAR}}")
endfunction()

function(generate_fast_build TARGET)
  get_target_property(TYPE ${TARGET} TYPE) #exe / sharedlib itp
  DBG(TYPE)
  get_target_property(LINKER_LANGUAGE ${TARGET} LINKER_LANGUAGE)
  DBG(LINKER_LANGUAGE)
  get_target_property(ARCHIVE_OUTPUT_DIRECTORY_CONFIG ${TARGET} ARCHIVE_OUTPUT_DIRECTORY_RELEASE)
  DBG(ARCHIVE_OUTPUT_DIRECTORY_CONFIG)
  get_target_property(COMPILE_OPTIONS ${TARGET} COMPILE_OPTIONS)
  DBG(COMPILE_OPTIONS)
  get_target_property(LINK_FLAGS_RELEASE ${TARGET} LINK_FLAGS_RELEASE)
  DBG(LINK_FLAGS_RELEASE)
  get_target_property(SOURCES ${TARGET} SOURCES)
  DBG(SOURCES)
  get_target_property(INTERFACE_COMPILE_OPTIONS ${TARGET} INTERFACE_COMPILE_OPTIONS)
  DBG(INTERFACE_COMPILE_OPTIONS)
  get_target_property(INCLUDE_DIRECTORIES ${TARGET} INCLUDE_DIRECTORIES)
  DBG(INCLUDE_DIRECTORIES)
  get_target_property(LINK_LIBRARIES ${TARGET} LINK_LIBRARIES)
  DBG(LINK_LIBRARIES)
  get_target_property(RESOURCE ${TARGET} RESOURCE)
  DBG(RESOURCE)
  
  set(IncludeDirs ${OpenCV_INCLUDE_DIRS})
  list(APPEND IncludeDirs "./include/")
  addPrefixes("${IncludeDirs}" " -I" IncludeDirs)
  string(REPLACE ";" "" IncludeDirs "${IncludeDirs}")
  addPrefixes("${OpenCV_LIBS}" " -l" IncludeLibs)
  addSuffixes("${IncludeLibs}" "341" IncludeLibs)
  string(REPLACE ";" "" IncludeLibs "${IncludeLibs}")
  set(libsPath " -L${OpenCV_INSTALL_PATH}/lib")
  toFastbuildArray("${SRC}" fba)


  file(WRITE fbuild.bff
  ".Compiler = '${CMAKE_CXX_COMPILER}' \n
  .CompilerOptions = '\"%1\"' 
  + ' -c' 
  + ' -o\"%2\"'
  + \"${IncludeDirs}${IncludeLibs}${libsPath}\"\n
  + ' -lstdc++fs'
  .Linker = '${CMAKE_CXX_COMPILER}' \n
  .LinkerOptions = ' \"%1\"' 
  + ' -o\"%2\"' 
  + \"${IncludeDirs}${IncludeLibs}${libsPath}\"\n
  + ' -lstdc++fs'
  ObjectList( '${PROJECT_NAME}-Lib' ) {
    .CompilerInputFiles = ${fba}
    .CompilerOutputPath = '/out/' 
  } 
  Executable( '${PROJECT_NAME}' ) { 
    .Libraries = { \"${PROJECT_NAME}-Lib\" } 
    .LinkerOutput = '/out/${PROJECT_NAME}.exe' 
  }
  Alias( 'all' ) { 
    .Targets = { '${PROJECT_NAME}' }
  }\n"
  )
endfunction()

