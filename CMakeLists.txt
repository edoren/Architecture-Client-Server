###############################################################################
## CMake configuration

cmake_minimum_required(VERSION 2.8.12)

# Set the custom module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Include the config file
include("${CMAKE_SOURCE_DIR}/cmake/Config.cmake")

# Limit the build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release"
    CACHE STRING "Limited configurations" FORCE)
mark_as_advanced(CMAKE_CONFIGURATION_TYPES)

###############################################################################
## Compiler configuration

# Adding C++11 support in GCC compiler and Clang compiler
if(${COMPILER} STREQUAL "Clang")
    if(${OS} STREQUAL "Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libstdc++")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
    endif()
elseif(${COMPILER} STREQUAL "GCC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# If compiler is GNU GCC or Clang enable the warnings
if(${COMPILER} STREQUAL "GCC" OR ${COMPILER} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

###############################################################################
## Directories configuration

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/bin")

###############################################################################
## Third party libraries configuration

# Find ZMQ
find_package(ZMQ)
if(NOT ZMQ_FOUND)
    message(STATUS "Building ZMQ from source.")
    add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/zmq" EXCLUDE_FROM_ALL)
    set(ZMQ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/zmq/include")
    set(ZMQ_LIBRARY libzmq)
endif()

# Add CPPZMQ
set(CPPZMQ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/cppzmq")

# Add MessagePack
set(MESSAGE_PACK_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/third_party/msgpack-c/include")

# Find SFML
find_package(SFML 2 COMPONENTS system audio)
if(NOT SFML_FOUND)
    add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/SFML" EXCLUDE_FROM_ALL)
    set(SFML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/SFML/include")
    set(SFML_LIBRARIES sfml-main sfml-system sfml-audio)
endif()

include_directories(SYSTEM
    ${ZMQ_INCLUDE_DIR}
    ${CPPZMQ_INCLUDE_DIR}
    ${MESSAGE_PACK_INCLUDE_DIR}
    ${SFML_INCLUDE_DIR})

###############################################################################
## Project configuration

add_subdirectory(src)
