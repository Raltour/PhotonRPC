# -------- Protobuf 库 --------
find_package(Protobuf REQUIRED)

# -------- PhotonRPC --------
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_library(photonrpc STATIC ${SOURCES})

target_include_directories(photonrpc PUBLIC ${PROJECT_SOURCE_DIR}/third_party)

target_link_libraries(photonrpc PRIVATE tinyxml2 protobuf::libprotobuf)