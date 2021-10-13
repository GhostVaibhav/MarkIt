cmake_minimum_required (VERSION 3.8)

include(ExternalProject)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE_BUILD_TYPE_INIT "MinSizeRel")
set(CMAKE_CONFIGURATION_TYPES "MinSizeRel")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR “In-source build detected!”)
endif()

project ("MarkIt")

add_subdirectory(deps/tabulate)

if(DEFINED COMPILE)
else()
	message(FATAL_ERROR "You should supply a value to \"COMPILE\" variable for processing, using -DCOMPILE=(0 or 1).0 for building the dependencies and 1 for building the project.")
endif()

if(${COMPILE})
else()
	ExternalProject_Add(
		curl_project
		URL "https://github.com/curl/curl/releases/download/curl-7_79_1/curl-7.79.1.zip"
		CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
		-DBUILD_CURL_EXE=OFF
		-DBUILD_SHARED_LIBS=ON
		-DBUILD_TESTING=OFF
		-DCMAKE_USE_SCHANNEL=ON
		-DUSE_WIN32_LDAP=ON
		-DENABLE_THREADED_RESOLVER=OFF
		-DUSE_LIBIDN2=OFF
	)
	ExternalProject_Add(
		pdcurses_project
		URL "https://github.com/GhostVaibhav/PDCurses/archive/refs/tags/4.0.zip"
		INSTALL_COMMAND ""
		CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
	)
	FetchContent_Declare(json
		GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
		GIT_TAG v3.10.3
	)
	FetchContent_GetProperties(json)
	if(NOT json_POPULATED)
		FetchContent_Populate(json)
		add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
	endif()
	unset(COMPILE CACHE)
	message(STATUS "Now put COMPILE=1 to build the main project")
endif()

if(${COMPILE})
	add_executable (${PROJECT_NAME} "src/main.cpp" "include/sha256.cpp")
	find_library(CURL_LIBRARY
		NAMES curl curllib libcurl_imp curllib_static
		HINTS "${CMAKE_BINARY_DIR}/lib"
		REQUIRED
	)
	find_library(PDCURSES_LIBRARY
		NAMES libpdcurses pdcurses curses
		HINTS "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/MinSizeRel/" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/Debug/" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/RelWithDebInfo/" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/Release/"
		REQUIRED
	)
	include_directories("include" "${CMAKE_BINARY_DIR}/include" "${nlohmann_json_SOURCE_DIR}/include/nlohmann" "${MarkIt_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project/" "${tabulate_SOURCE_DIR}/single_include/tabulate/")
	target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY} ${PDCURSES_LIBRARY} tabulate)
	unset(COMPILE CACHE)
endif()