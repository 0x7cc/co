cmake_minimum_required ( VERSION 3.13 )

project ( co )
set ( CMAKE_C_STANDARD 99 )
set ( CMAKE_CXX_STANDARD 11 )

include ( GNUInstallDirs )
include ( TestBigEndian )

set ( CMAKE_DEBUG_POSTFIX "d" )
set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/ )
set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/ )
set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug/ )
set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug/ )
set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release/ )
set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release/ )

add_custom_command (
  OUTPUT ${PROJECT_SOURCE_DIR}/co.o
  DEPENDS ${PROJECT_SOURCE_DIR}/co.asm
  COMMAND nasm -f elf64 ${PROJECT_SOURCE_DIR}/co.asm -g
  COMMENT "nasm generate co file"
)

#####################################

add_library ( co
  co.c
  co.h
  co_win.c
  co_x.c
  ${PROJECT_SOURCE_DIR}/co.o
  )

target_include_directories ( co
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  )
#####################################

add_executable ( test
  test.c
  )

target_include_directories ( test
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  )

target_link_libraries ( test PUBLIC co pthread )


function ( dump_cmake_variables )
  get_cmake_property ( _variableNames VARIABLES )
  list ( SORT _variableNames )
  foreach ( _variableName ${_variableNames} )
    if ( ARGV0 )
      unset ( MATCHED )
      string ( REGEX MATCH ${ARGV0} MATCHED ${_variableName} )
      if ( NOT MATCHED )
        continue ()
      endif ()
    endif ()
    message ( STATUS "${_variableName}=${${_variableName}}" )
  endforeach ()
endfunction ()

# dump_cmake_variables ()
