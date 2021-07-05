cmake_minimum_required(VERSION 2.8.12)

project(Todo)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(lib)

include(ExternalProject)
ExternalProject_Add(
    Curl
    GIT_REPOSITORY "https://github.com/curl/curl.git"
    CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DBUILD_CURL_EXE=OFF -DBUILD_TESTING=OFF -DCMAKE_USE_SCHANNEL=ON -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_INSTALL_PREFIX=""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)
include(ExternalProject)
ExternalProject_Add(
    PDCurses
    GIT_REPOSITORY "https://github.com/Bill-Gray/PDCursesMod.git"
    CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DPDC_BUILD_SHARED=OFF -DPDC_SDL2_BUILD=OFF -DCHTYPE_32=ON -DPDC_SDL2_DEPS_BUILD=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_INSTALL_PREFIX=""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)
add_executable(${PROJECT_NAME} main.cpp include/sha256.cpp)

add_dependencies(Todo Curl PDCurses)
find_library(CURL_LIB libcurl HINTS D:/Folders/Todo/lib)
find_library(PDCURSES_LIB pdcurses HINTS D:/Folders/Todo/lib)
target_link_libraries(${PROJECT_NAME} ${CURL_LIB} ${PDCURSES_LIB})
