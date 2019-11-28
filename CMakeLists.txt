cmake_minimum_required(VERSION 3.10)
project(deferred VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

if(APPLE)
  find_library(COCOA_LIBS Cocoa REQUIRED)
  find_library(IOKIT_LIBS IOKit REQUIRED)
  find_library(CORE_LIBS CoreVideo REQUIRED)
endif(APPLE)

if(LINUX)
  find_library(X11_LIBRARY X11 REQUIRED)
endif(LINUX)

if(WIN32)
  # find win32 api
endif(WIN32)

find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
  message(STATUS ">>> OpenGL not found")
endif(NOT OpenGL_FOUND)

find_package(GLEW REQUIRED)
if(NOT GLEW_FOUND)
  message(STATUS ">>> GLEW not found")
endif(NOT GLEW_FOUND)

find_package(glfw3 REQUIRED)
if(NOT GLFW3_FOUND)
  message(STATUS ">>> GLFW not found")
endif(NOT GLFW3_FOUND)

find_package(GLM REQUIRED)
if(NOT GLM_FOUND)
  message(STATUS ">>> GLM not found")
endif(NOT GLM_FOUND)

find_package(Freetype REQUIRED)
if(NOT FREETYPE_FOUND)
  message(Error  "Freetype not found")
endif(NOT FREETYPE_FOUND)

set(THIRD_PARTY_INCLUDE_DIRS "third_party/")
set(SOURCE_PREFIX "src")
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_PREFIX}/*.h ${SOURCE_PREFIX}/*.cpp)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
include_directories(${THIRD_PARTY_INCLUDE_DIRS})

if(OPENGL_FOUND AND GLEW_FOUND AND GLFW3_FOUND)
  # GLFW3_INCLUDE_DIR is /usr/local/include that includes GLFW, GL (GLEW) include dir
  set(EXTRA_INCLUDE_DIR ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
  target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${EXTRA_INCLUDE_DIR})

  if(APPLE)
    message(STATUS "DETECTED: platform macos")
    set(GLEW_LIBS "/usr/local/lib/libGLEW.a")
    set(EXTRA_LIB_DIR ${COCOA_LIBS} ${IOKIT_LIBS} ${CORE_LIBS} ${GLEW_LIBS} ${OPENGL_gl_LIBRARY} ${GLFW3_LIBRARY} ${FREETYPE_LIBRARY})
  elseif(LINUX)
    message(STATUS "DETECTED: platform linux")
    set(EXTRA_LIB_DIR ${X11_LIBRARY} ${OPENGL_gl_LIBRARY} ${GLEW} ${GLFW3_LIBRARY} ${FREETYPE_LIBRARY})
  elseif(WIN32)
    message(STATUS "DETECTED: platform windows")
  endif()
  target_link_libraries(${CMAKE_PROJECT_NAME} ${EXTRA_LIB_DIR})
endif()

# print all of cmake variables to terminal
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
