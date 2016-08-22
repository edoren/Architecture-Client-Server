include_directories(${THIRD_PARTY_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

###############################################################################
## 1 - Operations

add_executable(Operations_Server "1_Operations/server.cpp")
add_executable(Operations_Client "1_Operations/client.cpp")
target_link_libraries(Operations_Server ${THIRD_PARTY_LIBRARIES})
target_link_libraries(Operations_Client ${THIRD_PARTY_LIBRARIES})

###############################################################################
## 2 - MatrixOps

add_executable(MatrixOps_Server "2_MatrixOps/server.cpp")
add_executable(MatrixOps_Client "2_MatrixOps/client.cpp")
target_link_libraries(MatrixOps_Server ${THIRD_PARTY_LIBRARIES})
target_link_libraries(MatrixOps_Client ${THIRD_PARTY_LIBRARIES})
