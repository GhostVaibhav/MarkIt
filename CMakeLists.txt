cmake_minimum_required(VERSION 2.8.12)

project(Todo)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(lib)

add_executable(${PROJECT_NAME} main.cpp include/sha256.cpp)

find_library(CURL_LIB libcurl.lib lib)
find_library(PDCURSES_LIB pdcurses.lib lib)
target_link_libraries(${PROJECT_NAME} ${CURL_LIB} ${PDCURSES_LIB})