cmake_minimum_required(VERSION 3.15)
project(PhotonRPC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

if (ENABLE_ASAN)
    message(STATUS "build with ASAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif ()

if (ENABLE_UBSAN)
    message(STATUS "build with UBSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif ()


include_directories(${PROJECT_SOURCE_DIR}/include)

# -------- TinyXML-2 库 --------
add_library(tinyxml2 STATIC ${PROJECT_SOURCE_DIR}/third_party/tinyxml2/tinyxml2.cpp)

target_include_directories(tinyxml2 PUBLIC ${PROJECT_SOURCE_DIR}/third_party/tinyxml2)

# -------- Spdlog 库 --------
add_subdirectory(third_party/spdlog)

# -------- Google-Test 库 --------
enable_testing()

add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)

## -------- Google Test --------
#include(FetchContent)
#
#FetchContent_Declare(
#        googletest
#        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
#)
#FetchContent_MakeAvailable(googletest)
#
#enable_testing()

# -------- PhotonRPC --------
add_subdirectory(src)

# -------- TestExample --------
add_subdirectory(test)
