cmake_minimum_required(VERSION 3.15)
project(PhotonRPC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)

# -------- TinyXML-2 库 --------
add_library(tinyxml2 STATIC ${PROJECT_SOURCE_DIR}/third_party/tinyxml2/tinyxml2.cpp)

target_include_directories(tinyxml2 PUBLIC ${PROJECT_SOURCE_DIR}/third_party/tinyxml2)

# -------- PhotonRPC --------
add_subdirectory(src)

# -------- TestExample --------
add_subdirectory(test)
