cmake_minimum_required(VERSION 3.2)
project(TinyRPC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# -------- TinyXML-2 库 --------
# third_party/tinyxml2/tinyxml2.cpp 编译成一个静态库
add_library(tinyxml2 STATIC ${PROJECT_SOURCE_DIR}/third_party/tinyxml2/tinyxml2.cpp)

target_include_directories(tinyxml2 PUBLIC ${PROJECT_SOURCE_DIR}/third_party/tinyxml2)


# -------- TinyRPC --------
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(RPC ${SOURCES})

target_include_directories(RPC PUBLIC ${PROJECT_SOURCE_DIR}/third_party)
target_link_libraries(RPC tinyxml2)
