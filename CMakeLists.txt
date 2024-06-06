cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE ${CMAKE_COMMAND})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR “${BoldRed}In-source build detected!${ColourReset}”)
endif()

if(NOT DEFINED ${CMAKE_BUILD_TYPE})
	set(CMAKE_BUILD_TYPE "Release")
endif()

project ("MarkIt")

file(GLOB srcs "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/utils/*.cpp")

if(WIN32)
  find_library (
    CURL_LIBRARY
    NAMES libcurl libcurl_imp
    HINTS "${CMAKE_SOURCE_DIR}/deps/curl/library/lib"
    REQUIRED
  )
else()
	find_package(
    CURL
    REQUIRED
  )
endif()
if(WIN32)
  find_library (
    CURSES_LIBRARY
    NAMES pdcurses
    HINTS "${CMAKE_SOURCE_DIR}/deps/PDCurses/library/lib"
    REQUIRED
  )
else()
	find_package(
    Curses
    REQUIRED
  )
endif()

add_executable (${PROJECT_NAME} ${srcs})

if(WIN32)
  target_include_directories(${PROJECT_NAME} PRIVATE "include"
                            "${CMAKE_SOURCE_DIR}/deps/curl/library/include"
                            "${CMAKE_SOURCE_DIR}/deps/tabulate/single_include/tabulate"
                            "${CMAKE_SOURCE_DIR}/deps/json/single_include/nlohmann"
                            "${CMAKE_SOURCE_DIR}/deps/PDCurses/library/include"
                            "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
                          )
else()
  target_include_directories(${PROJECT_NAME} PRIVATE "include"
                            "${CMAKE_SOURCE_DIR}/deps/tabulate/single_include/tabulate"
                            "${CMAKE_SOURCE_DIR}/deps/json/single_include/nlohmann"
                            ${CURSES_INCLUDE_DIR}
                            "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
                          )
endif()

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARY} ${CURSES_LIBRARY})
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl ${CURSES_LIBRARIES})
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_SOURCE_DIR}/deps/curl/library/bin"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
                  )
endif()
