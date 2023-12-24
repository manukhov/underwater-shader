cmake_minimum_required(VERSION 3.26)

include(FetchContent)

set(CMAKE_GENERATOR_PLATFORM Win32)

project(underwater_anticheat)

add_library(${PROJECT_NAME} MODULE)

target_sources(${PROJECT_NAME} PRIVATE shader.cpp shader.hpp)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")

add_definitions(-DUNICODE -D_UNICODE)
 
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".asi")