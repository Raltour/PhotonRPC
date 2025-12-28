include_directories(${CMAKE_CURRENT_SOURCE_DIR}/protocol)
file(GLOB PROTO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/protocol/*.cc)

# -------- TestClient --------
add_executable(TestClient ${PROJECT_SOURCE_DIR}/test/test_client.cc ${PROTO_SOURCES} ${PROJECT_SOURCE_DIR}/src/protocol/rpc_message.pb.cc)
target_link_libraries(TestClient PRIVATE photonrpc)

# -------- TestServer --------
add_executable(TestServer ${PROJECT_SOURCE_DIR}/test/test_server.cc ${PROTO_SOURCES}  ${PROJECT_SOURCE_DIR}/src/protocol/rpc_message.pb.cc)
target_link_libraries(TestServer PRIVATE photonrpc)