cmake_minimum_required(VERSION 3.20)

include(FetchContent)

cmake_policy(SET CMP0074 NEW)

set(SDL_REQUIRED_VERSION 3.0.0)
set(C_STANDARD 99)


set(MAJOR_VERSION 1)
set(MINOR_VERSION 0)
set(MICRO_VERSION 0)
project(Napys
    LANGUAGES C CXX
    VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}"
)

FetchContent_Declare(
    SDL3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
    GIT_TAG "release-3.2.16"
)
FetchContent_MakeAvailable(SDL3)


set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3_ttf
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL_ttf"
    GIT_TAG "release-3.2.2"
)
FetchContent_MakeAvailable(SDL3_ttf)

set(LIB_SOURCES 
    src/napys.c
)

add_library(
    Napys
    STATIC
    ${LIB_SOURCES}
)

target_link_libraries(Napys PUBLIC SDL3::SDL3 SDL3_ttf::SDL3_ttf)

target_include_directories(Napys PUBLIC include/)

option(NAPYS_BUILD_EXAMPLES "Build Napys examples" ON)

if (NAPYS_BUILD_EXAMPLES)
    add_subdirectory(examples/)
endif()