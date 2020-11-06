cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()
if (POLICY CMP0028)
  cmake_policy(SET CMP0028 NEW)
endif()
if (POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()
if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

project(version-history-api)

# Enable C++17 features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)
set(OPENSSL_USE_STATIC_LIBS TRUE)

find_package(libmongocxx-static REQUIRED)
find_package(Threads)
find_package(Boost REQUIRED COMPONENTS iostreams)
if (APPLE)
  find_package(OpenSSL REQUIRED)
endif()

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

include_directories(
  ${LIBMONGOCXX_STATIC_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)

if(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
  LINK_DIRECTORIES(
    /opt/local/lib
  )
else()
  set(CMAKE_MACOSX_RPATH 1)
  include_directories(
    /usr/local/nghttp2/include
  )
  LINK_DIRECTORIES(
    /usr/local/nghttp2/lib
  )
endif()

SET(CMAKE_INSTALL_PREFIX /opt/spt)

add_subdirectory(src)

if (BUILD_TESTING)
  enable_testing()
  add_subdirectory(test/integration)
endif()

