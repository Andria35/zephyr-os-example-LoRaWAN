# Install script for directory: /Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/opt/homebrew/opt/arm-none-eabi-gcc/bin/arm-none-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/andriasworld/Programming/iot-engineering/zephyr‐os‐example‐LoRaWAN/build/modules/picolibc/newlib/libc/include/sys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/andriasworld/Programming/iot-engineering/zephyr‐os‐example‐LoRaWAN/build/modules/picolibc/newlib/libc/include/machine/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/andriasworld/Programming/iot-engineering/zephyr‐os‐example‐LoRaWAN/build/modules/picolibc/newlib/libc/include/ssp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/andriasworld/Programming/iot-engineering/zephyr‐os‐example‐LoRaWAN/build/modules/picolibc/newlib/libc/include/rpc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/andriasworld/Programming/iot-engineering/zephyr‐os‐example‐LoRaWAN/build/modules/picolibc/newlib/libc/include/arpa/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/alloca.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/argz.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/ar.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/assert.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/byteswap.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/cpio.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/ctype.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/devctl.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/dirent.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/endian.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/envlock.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/envz.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/errno.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/fastmath.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/fcntl.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/fenv.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/fnmatch.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/getopt.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/glob.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/grp.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/iconv.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/ieeefp.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/inttypes.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/langinfo.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/libgen.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/limits.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/locale.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/malloc.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/math.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/memory.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/ndbm.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/newlib.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/paths.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/picotls.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/pwd.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/regdef.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/regex.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/sched.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/search.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/setjmp.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/signal.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/spawn.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/stdint.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/stdnoreturn.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/stdlib.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/string.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/strings.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/tar.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/termios.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/time.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/uchar.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/unctrl.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/unistd.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/utime.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/utmp.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/wchar.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/wctype.h"
    "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/wordexp.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/Users/andriasworld/zephyrproject/modules/lib/picolibc/newlib/libc/include/complex.h")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/andriasworld/Programming/iot-engineering/zephyr‐os‐example‐LoRaWAN/build/modules/picolibc/newlib/libc/include/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
