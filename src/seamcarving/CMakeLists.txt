cmake_minimum_required(VERSION 3.15)
project(SeamCarving)

set(SRC
    main.cpp
    seamcarving.cpp
    seamcarving.h
    cvmatrenderer.cpp cvmatrenderer.h)

add_executable(${PROJECT_NAME} ${SRC})

find_package(OpenMP REQUIRED)


# Where to find CMake modules and OpenCV
set(OpenCV_DIR "C:/opencv-4.1.1/mingw-build/install_release")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
#include(MacroOpenCV)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


if (OpenMP_CXX_FOUND)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS} IGSE OpenMP::OpenMP_CXX)
else ()
  target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS} IGSE)
endif ()