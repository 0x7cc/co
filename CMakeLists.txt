cmake_minimum_required ( VERSION 2.8 )

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

if ( CMAKE_HOST_UNIX )
  set ( NASM_OBJ ${CMAKE_CURRENT_BINARY_DIR}/co.o )
elseif ( CMAKE_HOST_WIN32 )
  set ( NASM_OBJ ${CMAKE_CURRENT_BINARY_DIR}/co.obj )
endif ()

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/co.o
  DEPENDS src/co/co.asm
  COMMAND nasm -f elf64 -o ${CMAKE_CURRENT_BINARY_DIR}/co.o ${CMAKE_CURRENT_LIST_DIR}/src/co/co.asm
  COMMENT "nasm generate elf64 file"
)

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/co.obj
  DEPENDS src/co/co.asm
  COMMAND nasm -f win64 -o ${CMAKE_CURRENT_BINARY_DIR}/co.obj ${CMAKE_CURRENT_LIST_DIR}/src/co/co.asm
  COMMENT "nasm generate win64 file"
)

#####################################

add_library ( co
  STATIC
  include/co/co.h
  src/co/co.c
  src/co/co_win.c
  src/co/co_x.c
  ${NASM_OBJ}
  )

target_include_directories ( co
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/include
  )

if ( target_type STREQUAL "EXECUTABLE" )
  # Process executable target
endif ()

if ( CMAKE_HOST_UNIX )
  target_link_libraries ( co PUBLIC pthread )
endif ()

#if ( MSVC )
#  set_target_properties ( co PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_LIST_DIR}/co/co.def\" " )
#endif ()

#####################################

add_executable ( testc
  tests/test.c
  )

target_include_directories ( testc
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/include
  )

target_link_libraries ( testc PUBLIC co )

#####################################

add_executable ( testcpp
  tests/test.cpp
  )

target_include_directories ( testcpp
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/include
  )

target_link_libraries ( testcpp PUBLIC co )

#####################################

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

#dump_cmake_variables ()
