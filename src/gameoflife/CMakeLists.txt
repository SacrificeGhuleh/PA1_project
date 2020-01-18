cmake_minimum_required(VERSION 3.15)
project(GameOfLife)

set(SRC
    main.cpp
    gameoflife.cpp
    gameoflife.h)

add_executable(${PROJECT_NAME} ${SRC})

find_package(OpenMP REQUIRED)

if (OpenMP_CXX_FOUND)
  target_link_libraries(${PROJECT_NAME} PUBLIC IGSE OpenMP::OpenMP_CXX)
else ()
  target_link_libraries(${PROJECT_NAME} PUBLIC IGSE)
endif ()