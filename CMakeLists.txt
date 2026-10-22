cmake_minimum_required(VERSION 3.21)

project(VETER VERSION 1.2.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK is not set. Install the Geode SDK and use the Geode build workflow.")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

if (NOT SOURCES)
    message(FATAL_ERROR "No C++ source files found. Make sure the mod source is at src/main.cpp.")
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES})

add_subdirectory($ENV{GEODE_SDK} "${CMAKE_CURRENT_BINARY_DIR}/geode")
setup_geode_mod(${PROJECT_NAME})

