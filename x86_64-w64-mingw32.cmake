
set ( CMAKE_SYSTEM_NAME Windows )

set ( CMAKE_AR x86_64-w64-mingw32-ar )
set ( CMAKE_LINKER x86_64-w64-mingw32-ld )
set ( CMAKE_NM x86_64-w64-mingw32-nm )
set ( CMAKE_OBJCOPY x86_64-w64-mingw32-objcopy )
set ( CMAKE_OBJDUMP x86_64-w64-mingw32-objdump )
set ( CMAKE_RANLIB x86_64-w64-mingw32-ranlib )
set ( CMAKE_RC_COMPILER x86_64-w64-mingw32-windres )
set ( CMAKE_STRIP x86_64-w64-mingw32-strip )

if ( CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" )

  # 根据测试发现 Ubuntu 下 cmake-3.13.0 之前的版本会出现 g++ 命令检查失败的情况.
  cmake_minimum_required ( VERSION 3.13.0 )

  set ( CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix )
  set ( CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix ) # 不带posix后缀的没有std::mutex....

  # Ubuntu 下的 mingw64 与 Windows 的不太一样,不能设置这些.

  set ( CMAKE_SYSROOT /usr/x86_64-w64-mingw32/ )
  set ( CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32/ )
  set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
  set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
  set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
  set ( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

elseif ( CMAKE_HOST_SYSTEM_NAME STREQUAL "CYGWIN" )

  set ( CMAKE_C_COMPILER x86_64-w64-mingw32-gcc )
  set ( CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++ )

endif ()

