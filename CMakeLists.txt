cmake_minimum_required(VERSION 3.20)
project(shivers C)

set(CMAKE_EXPORT_COMPILE_COMMANDS yes)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT supported OUTPUT error)

  if(supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO/IPO optimization is enabled.")
  else()
    message(WARNING "LTO/IPO optimization is not supported: ${error}")
  endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR}
    CACHE STRING "Target architecture for macOS"
    FORCE
  )

  list(APPEND CMAKE_PREFIX_PATH
    "/opt/homebrew"
    "/usr/local"
  )

  set(MATH_LIB "")
  set(LINK_FLAGS "-Wl-bind_at_load")
else()
  set(MATH_LIB m)
  set(LINK_FLAGS "-Wl,-z,now")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
pkg_check_modules(FREETYPE REQUIRED QUIET freetype2)


file(GLOB SOURCES "libs/**/*.c" "src/**/*.c" "src/*.c")


# shivers target
add_executable(shivers ${SOURCES})
set_property(TARGET shivers PROPERTY C_STANDARD 23)
set_property(TARGET shivers PROPERTY C_STANDARD_REQUIRED ON)

target_link_options(shivers PRIVATE ${LINK_FLAGS})


target_include_directories(shivers PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(shivers PRIVATE
  OpenSSL::SSL
  OpenSSL::Crypto
  Threads::Threads
  ZLIB::ZLIB
  ${FREETYPE_LINK_LIBRARIES}
  ${MATH_LIB}
)

target_compile_definitions(shivers
  PUBLIC _TIME_BITS=64
  PUBLIC _GNU_SOURCE
  PUBLIC _FILE_OFFSET_BITS=64
)

install(TARGETS shivers)
