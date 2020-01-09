cmake_minimum_required(VERSION 3.15)
project(PA1_Project)

set(CMAKE_CXX_STANDARD 17)

set(LIBS_ROOT libs)
set(IGSE_ROOT ${LIBS_ROOT}/IGSE)

add_subdirectory(${IGSE_ROOT})

add_executable(PA1_Project src/main.cpp)

target_link_libraries(PA1_Project PUBLIC IGSE)