cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE ${CMAKE_COMMAND})
set(CMAKE_MAJOR_VERSION 0)
set(CMAKE_MINOR_VERSION 1)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR “${BoldRed}In-source build detected!${ColourReset}”)
endif()

if(DEFINED CMAKE_BUILD_GUI)
  set(CMAKE_BUILD_GUI ${CMAKE_BUILD_GUI})
else()
  set(CMAKE_BUILD_GUI 1)
endif()

if(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
else()
  set(CMAKE_BUILD_TYPE "Debug")
endif()

project ("MarkIt")

configure_file(
  "${PROJECT_SOURCE_DIR}/include/toggles.h.in"
  "${PROJECT_SOURCE_DIR}/include/toggles.h"
)

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
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
  target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl ${CURSES_LIBRARIES})
endif()

if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_SOURCE_DIR}/deps/curl/library/bin"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
                  )
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT Application)
if(WIN32)
  install(FILES "${PROJECT_SOURCE_DIR}/deps/curl/library/bin/libcurl.dll" DESTINATION bin COMPONENT Application)
endif()

include(InstallRequiredSystemLibraries)

if(WIN32)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
  set(CPACK_COMPONENTS_ALL Application)
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_COMPONENTS_ALL Application)
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
  set(CPACK_DMG_VOLUME_NAME "${CMAKE_PROJECT_NAME}")
else()
  set(CPACK_GENERATOR "DEB")
  set(CPACK_COMPONENTS_ALL Application)
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vaibhav Sharma")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4-openssl-dev, libncurses5-dev")
endif()

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VENDOR "Vaibhav Sharma")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")

include(CPack)
