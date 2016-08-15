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

# Find ZMQ and ZMQPP
find_package(ZMQ)
find_package(ZMQPP)
if (ZMQ_FOUND AND ZMQPP_FOUND)
    set(THIRD_PARTY_LIBRARIES ${ZMQ_LIBRARIES} ${ZMQPP_LIBRARIES})
    set(THIRD_PARTY_INCLUDE_DIRS ${ZMQ_INCLUDE_DIRS} ${ZMQPP_INCLUDE_DIRS})
else()
    message(STATUS "Building ZMQPP from source.")
    set(ZMQPP_LIBZMQ_CMAKE true)
    set(ZEROMQ_INCLUDE "${CMAKE_SOURCE_DIR}/third_party/zmq/include")
    set(ZMQPP_INCLUDE "${CMAKE_SOURCE_DIR}/third_party/zmqpp/src")
    add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/zmq" EXCLUDE_FROM_ALL)
    set(ZEROMQ_LIBRARY_SHARED libzmq)
    set(ZEROMQ_LIBRARY_STATIC libzmq-static)
    add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/zmqpp" EXCLUDE_FROM_ALL)
    set(THIRD_PARTY_LIBRARIES libzmq zmqpp)
    set(THIRD_PARTY_INCLUDE_DIRS ${ZEROMQ_INCLUDE} ${ZMQPP_INCLUDE})
endif()

###############################################################################
## Project configuration

set(SERVER_SOURCE
    "${CMAKE_SOURCE_DIR}/src/simple_server.cpp"
)

set(CLIENT_SOURCE
    "${CMAKE_SOURCE_DIR}/src/simple_client.cpp"
)

add_executable(server ${SERVER_SOURCE})
add_executable(client ${CLIENT_SOURCE})

include_directories(${THIRD_PARTY_INCLUDE_DIRS})

target_link_libraries(server ${THIRD_PARTY_LIBRARIES})
target_link_libraries(client ${THIRD_PARTY_LIBRARIES})
