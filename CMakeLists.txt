cmake_minimum_required(VERSION 3.22.1)
project(BizierEditor)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/build/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build/)

#include_directories($ENV{EMSDK}/upstream/emscripten/cache/sysroot/include)

add_executable(BizierEditor main.cpp
        src/base/base.h
        src/base/base.cpp
        src/base/grid.h
        src/base/grid.cpp
        src/base/instance.h
        src/base/instance.cpp
        src/base/PickingTexture.cpp
        src/base/PickingTexture.h
        src/base/BizierCurve.cpp
        src/base/BizierCurve.h
)

add_subdirectory(imgui)
target_link_libraries(BizierEditor PUBLIC imgui)
include_directories("${PROJECT_SOURCE_DIR}/imgui")

if (DEFINED $ENV{EMSDK})
    message(FATAL_ERROR "Please set EMSDK environment variable")
endif ()

set(MY_LINK_FLAGS " --preload-file ${PROJECT_SOURCE_DIR}/shaders@/shaders -s MIN_WEBGL_VERSION=2 -s USE_WEBGL2=1 -s  -s USE_SDL_IMAGE=2 -s USE_SDL=2")


IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_BUILD_TYPE Debug)
    set(ENV{EMCC_DEBUG} 1)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(MY_LINK_FLAGS "${MY_LINK_FLAGS} -g --memoryprofiler")
    add_compile_options(-g)
ENDIF ()

set_target_properties(BizierEditor PROPERTIES LINK_FLAGS ${MY_LINK_FLAGS})