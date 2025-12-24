# -------- TestClient --------
add_executable(TestClient ${PROJECT_SOURCE_DIR}/test/test_client.cc ${PROJECT_SOURCE_DIR}/src/protocol/add_service.pb.cc ${PROJECT_SOURCE_DIR}/src/protocol/rpc_message.pb.cc)
target_link_libraries(TestClient PRIVATE tinyrpc)

# -------- TestServer --------
add_executable(TestServer ${PROJECT_SOURCE_DIR}/test/test_server.cc ${PROJECT_SOURCE_DIR}/src/protocol/add_service.pb.cc ${PROJECT_SOURCE_DIR}/src/protocol/rpc_message.pb.cc)
target_link_libraries(TestServer PRIVATE tinyrpc)