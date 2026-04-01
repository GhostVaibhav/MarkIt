cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE ${CMAKE_COMMAND})
set(CMAKE_MAJOR_VERSION 0)
set(CMAKE_MINOR_VERSION 1)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR “${BoldRed}In-source build detected!${ColourReset}”)
endif()

if(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
else()
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CMAKE_DEBUG_BUILD 1)
else()
  set(CMAKE_DEBUG_BUILD 0)
endif()

project ("MarkIt")

configure_file(
  "${PROJECT_SOURCE_DIR}/include/toggles.h.in"
  "${PROJECT_SOURCE_DIR}/include/toggles.h"
)

# SQLiteCpp configuration
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "Run cpplint.py on sources" FORCE)
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "Run cppcheck on sources" FORCE)
set(SQLITECPP_RUN_DOXYGEN OFF CACHE BOOL "Run Doxygen C++ documentation compiler" FORCE)
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(SQLITECPP_INSTALL OFF CACHE BOOL "Install SQLiteCpp target" FORCE)
set(SQLITECPP_USE_STACK_PROTECTION OFF CACHE BOOL "Use stack protection" FORCE)
add_subdirectory(deps/SQLiteCpp)
if (WIN32)
add_subdirectory(deps/PDCurses)
add_subdirectory(deps/curl)
endif()
add_subdirectory(deps/spdlog)
add_subdirectory(deps/json)

file(GLOB srcs
          "${PROJECT_SOURCE_DIR}/src/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/models/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/config/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/files/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/db/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/managers/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/commands/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/panels/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/online/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/app/*.cpp"
          "${PROJECT_SOURCE_DIR}/src/utils/*.cpp"
)

if(WIN32)
else()
	find_package(
    CURL
    REQUIRED
  )
endif()
if(WIN32)
else()
	find_package(
    Curses
    REQUIRED
  )
endif()

add_executable (${PROJECT_NAME} ${srcs})

target_include_directories(${PROJECT_NAME} PRIVATE 
    "include"
    "include/models"
    "include/config"
    "include/files"
    "include/db"
    "include/managers"
    "include/commands"
    "include/panels"
    "include/online"
    "include/app"
    "include/enums"
    "${CMAKE_SOURCE_DIR}/deps/tabulate/single_include/tabulate"
    "${CMAKE_SOURCE_DIR}/deps/json/single_include/nlohmann"
    "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
)

if(WIN32)
  target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_SOURCE_DIR}/deps/curl/include"
    "${CMAKE_SOURCE_DIR}/deps/PDCurses"
  )
else()
  target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CURSES_INCLUDE_DIR}
  )
endif()

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl pdcurses SQLiteCpp)
else()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads CURL::libcurl ${CURSES_LIBRARIES} SQLiteCpp)
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
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
  set(CPACK_DMG_VOLUME_NAME "${CMAKE_PROJECT_NAME}")
else()
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vaibhav Sharma")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4-openssl-dev, libncurses5-dev")
endif()

set(CPACK_COMPONENTS_ALL Application)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VENDOR "Vaibhav Sharma")
set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MINOR_VERSION})

include(CPack)
