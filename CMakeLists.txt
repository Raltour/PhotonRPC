cmake_minimum_required(VERSION 3.2)
project(TinyRPC)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(RPC ${SOURCES})
