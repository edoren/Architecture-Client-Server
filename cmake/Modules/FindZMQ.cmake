# - Try to find ZEROMQ http://zeromq.org
#
# Once done this will define:
#
#   ZMQ_FOUND        - System has ZMQ
#   ZMQ_INCLUDE_DIRS - The ZMQ include directory
#   ZMQ_LIBRARIES    - The ZMQ library
#
# It also defines:
#
#   ZMQ_INCLUDE_DIR - Same as ZMQ_INCLUDE_DIRS
#   ZMQ_LIBRARY     - Same as ZMQ_LIBRARIES
#

find_path(ZMQ_INCLUDE_DIR NAMES zmq.h)

find_library(ZMQ_LIBRARY NAMES zmq)

set(ZMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR})
set(ZMQ_LIBRARIES ${ZMQ_LIBRARY})

include(FindPackageHandleStandardArgs)

# Handle the QUIETLY and REQUIRED arguments and set ZMQ_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ZMQ DEFAULT_MSG
                                  ZMQ_LIBRARIES ZMQ_INCLUDE_DIRS)

mark_as_advanced(ZMQ_LIBRARIES ZMQ_INCLUDE_DIRS)
