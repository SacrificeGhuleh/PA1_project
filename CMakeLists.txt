cmake_minimum_required(VERSION 3.15)
project(PA1_Project)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


find_package(Doxygen)

if (DOXYGEN_FOUND)
  set(DOXYGEN_INPUT ${SOURCE_FILES})
  set(DOXYGEN_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  
  add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
    COMMAND ${CMAKE_COMMAND} -E echo "Done."
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${DOXYGEN_INPUT}
  )
  
  add_custom_target(apidoc ALL DEPENDS ${DOXYGEN_OUTPUT})
  
  add_custom_target(apidoc_forced
                    COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
                    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
                    COMMAND ${CMAKE_COMMAND} -E echo "Done."
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    )
endif (DOXYGEN_FOUND)

set(LIBS_ROOT libs)
set(SRC_ROOT src)
set(GAMEOFLIFE_ROOT ${SRC_ROOT}/gameoflife)
set(SEAMCARVING_ROOT ${SRC_ROOT}/seamcarving)

set(IGSE_ROOT ${LIBS_ROOT}/IGSE)

add_subdirectory(${IGSE_ROOT})

#TODO This should be loaded from add subdir igse
include_directories(${IGSE_ROOT}/include)
include_directories(${IGSE_ROOT}/libs/imgui)
include_directories(${IGSE_ROOT}/libs/imgui/examples)
include_directories(${IGSE_ROOT}/libs/gl3w/include)
include_directories(${IGSE_ROOT}/libs/glfw/include)

add_subdirectory(${GAMEOFLIFE_ROOT})
add_subdirectory(${SEAMCARVING_ROOT})
