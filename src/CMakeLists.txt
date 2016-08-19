###############################################################################
## 1 - Operations

add_executable(Operations_Server "1_Operations/server.cpp")
add_executable(Operations_Client "1_Operations/client.cpp")

include_directories(${THIRD_PARTY_INCLUDE_DIRS})

target_link_libraries(Operations_Server ${THIRD_PARTY_LIBRARIES})
target_link_libraries(Operations_Client ${THIRD_PARTY_LIBRARIES})

###############################################################################
## 2 - MatrixOps

