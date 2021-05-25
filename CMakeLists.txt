# CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(SRIX4K-Reader C)

# C11 standard
set(CMAKE_C_STANDARD 11)

# Set release if unset
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find libnfc
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNFC REQUIRED libnfc)
include_directories(${LIBNFC_INCLUDE_DIRS})
link_directories(${LIBNFC_LIBRARY_DIRS})
add_definitions(${LIBNFC_CFLAGS_OTHER})

# Optimization flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pipe")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -pipe")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -s")

# Compile mikai CLI executable
add_executable(SRIX4K-Reader main.c srix.c srixflag.c reader.c)
target_link_libraries(SRIX4K-Reader ${LIBNFC_LIBRARIES})