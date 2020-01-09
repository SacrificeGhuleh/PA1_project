cmake_minimum_required(VERSION 3.15)
project(PA1_Project)

set(CMAKE_CXX_STANDARD 17)

set(LIBS_ROOT libs)
set(SRC_ROOT src)
set(GAMEOFLIFE_ROOT ${SRC_ROOT}/gameoflife)
set(IGSE_ROOT ${LIBS_ROOT}/IGSE)

add_subdirectory(${IGSE_ROOT})

include_directories(${IGSE_ROOT}/include)
include_directories(${IGSE_ROOT}/libs/imgui)
include_directories(${IGSE_ROOT}/libs/imgui/examples)
include_directories(${IGSE_ROOT}/libs/gl3w/include)
include_directories(${IGSE_ROOT}/libs/glfw/include)

add_subdirectory(${GAMEOFLIFE_ROOT})
