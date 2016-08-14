# - Try to find ZMQPP http://zeromq.github.io/zmqpp/
#
# Once done this will define:
#
#   ZMQPP_FOUND - System has ZMQPP
#   ZMQPP_INCLUDE_DIRS - The ZMQPP include directory
#   ZMQPP_LIBRARIES - The ZMQPP library
#

find_path(ZMQPP_INCLUDE_DIR NAMES zmqpp/zmqpp.hpp)

find_library(ZMQPP_LIBRARY NAMES zmqpp)

set(ZMQPP_INCLUDE_DIRS ${ZMQPP_INCLUDE_DIR})
set(ZMQPP_LIBRARIES ${ZMQPP_LIBRARY})

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set ZMQPP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ZMQPP DEFAULT_MSG
                                  ZMQPP_LIBRARIES ZMQPP_INCLUDE_DIRS)

mark_as_advanced(ZMQPP_LIBRARIES ZMQPP_INCLUDE_DIRS)
