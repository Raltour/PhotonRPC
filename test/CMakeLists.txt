include_directories(${CMAKE_CURRENT_SOURCE_DIR}/protocol)
file(GLOB PROTO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/protocol/*.cc)

# -------- TestConsumer --------
add_executable(TestConsumer ${PROJECT_SOURCE_DIR}/test/consumer.cc ${PROTO_SOURCES})
target_link_libraries(TestConsumer PRIVATE photonrpc)

# -------- TestProvide --------
add_executable(TestProvider ${PROJECT_SOURCE_DIR}/test/provider.cc ${PROTO_SOURCES})
target_link_libraries(TestProvider PRIVATE photonrpc)

# -------- TestBuffer --------
add_executable(TestBuffer test_buffer.cc)
target_link_libraries(TestBuffer PRIVATE photonrpc GTest::gtest_main)

# ---------- TestCodec ----------
add_executable(TestCodec test_codec.cc)
target_link_libraries(TestCodec PRIVATE photonrpc GTest::gtest_main)

add_executable(TestThreadPool test_thread_pool.cc)
target_link_libraries(TestThreadPool PRIVATE photonrpc GTest::gtest_main)

add_executable(TestEventLoop test_event_loop.cc)
target_link_libraries(TestEventLoop PRIVATE photonrpc GTest::gtest_main)

#include(GoogleTest)
#gtest_discover_tests(TestBuffer)
add_test(NAME TestBuffer COMMAND TestBuffer)
add_test(NAME TestCodec COMMAND TestCodec)
add_test(NAME TestThreadPool COMMAND TestThreadPool)
add_test(NAME TestEventLoop COMMAND TestEventLoop)

# ---------- Benchmark ----------
add_executable(Benchmark benchmark.cc ${PROTO_SOURCES})
target_link_libraries(Benchmark PRIVATE photonrpc)
