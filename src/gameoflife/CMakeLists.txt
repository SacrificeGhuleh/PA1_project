cmake_minimum_required(VERSION 3.15)
project(GameOfLife)

set(SRC
    main.cpp
    gameoflife.cpp
    gameoflife.h)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(GameOfLife PUBLIC IGSE)