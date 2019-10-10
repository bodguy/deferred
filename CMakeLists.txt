cmake_minimum_required(VERSION 3.15)
project(deferred VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_library(COCOA_LIBS Cocoa REQUIRED)
find_library(IOKIT_LIBS IOKit REQUIRED)
find_library(CORE_LIBS CoreVideo REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
if(OPENGL_FOUND AND GLEW_FOUND)
    # GLFW3_INCLUDE_DIR is /usr/local/include that includes GLFW, GL (GLEW) include dir
    set(EXTRA_INCLUDE_DIR ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR})
    target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${EXTRA_INCLUDE_DIR})
    target_link_libraries(${CMAKE_PROJECT_NAME} ${COCOA_LIBS} ${IOKIT_LIBS} ${CORE_LIBS} ${OPENGL_gl_LIBRARY} ${GLEW} ${GLFW3_LIBRARY})
endif()

# print all of cmake variables to terminal
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()