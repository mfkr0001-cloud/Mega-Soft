
cmake_minimum_required(VERSION 3.21)

project(MegaSoft VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is missing")
endif()

add_library(${PROJECT_NAME} SHARED
    main.cpp
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src")
    file(GLOB_RECURSE EXTRA_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    )
    target_sources(${PROJECT_NAME} PRIVATE ${EXTRA_SOURCES})
endif()

add_subdirectory(
    "$ENV{GEODE_SDK}"
    "${CMAKE_CURRENT_BINARY_DIR}/geode"
)

setup_geode_mod(${PROJECT_NAME})

