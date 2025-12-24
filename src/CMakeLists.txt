# -------- Protobuf 库 --------
find_package(Protobuf REQUIRED)

# -------- TinyRPC --------
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_library(tinyrpc STATIC ${SOURCES})

target_include_directories(tinyrpc PUBLIC ${PROJECT_SOURCE_DIR}/third_party)

target_link_libraries(tinyrpc PRIVATE tinyxml2 protobuf::libprotobuf)