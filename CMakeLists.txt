#[[
 *      __  ___         __    ____ __  __
 *     /  |/  /__ _____/ /__ /  _// /_/ /
 *    / /|_/ / _ `/ __/  '_/_/ / / __/_/
 *   /_/  /_/\_,_/_/ /_/\_\/___/ \__(_)
 *   
 *   MIT License
 *   
 *   Copyright (c) 2021 Vaibhav Sharma
 *   
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *   
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 ]]

cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE_BUILD_TYPE_INIT "MinSizeRel")
set(CMAKE_CONFIGURATION_TYPES "MinSizeRel" "Debug")

include(ExternalProject)
include(FetchContent)
include(GNUInstallDirs)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR “In-source build detected!”)
endif()

project ("MarkIt")

add_subdirectory(deps/tabulate)

if(DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()

if(DEFINED COMPILE OR NOT WIN32)
else()
	message(FATAL_ERROR "You should supply a value to \"COMPILE\" variable for processing, using -DCOMPILE=(0 or 1). 0 for building the dependencies and 1 for building the project.")
endif()

if(${COMPILE} EQUAL 0)
	if(WIN32)
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
	else()
		if(APPLE)
		else()
			if((DEFINED INFOEX) AND (INFOEX EQUAL 1))
				set(CMAKE_C_FLAGS "-DHAVE_NO_INFOEX")
				set(CMAKE_CXX_FLAGS "-DHAVE_NO_INFOEX")
			else()
				set(INFOEX 0)
			endif()
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
				BUILD_COMMAND ../curl_project/configure --without-ssl --prefix=${CMAKE_BINARY_DIR}
				COMMAND ${CMAKE_COMMAND} --build .
				INSTALL_COMMAND ${CMAKE_COMMAND} --build . -- install
			)
		endif()
		ExternalProject_Add(
			ncurses_project
			URL "https://github.com/mirror/ncurses/archive/refs/tags/v6.2.zip"
			UPDATE_COMMAND ""
			CONFIGURE_COMMAND ""
			BUILD_COMMAND ../ncurses_project/configure --prefix=${CMAKE_BINARY_DIR} --disable-overwrite
			COMMAND make
			INSTALL_COMMAND make install
		)
	endif()
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
	if(WIN32)
		message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
		message(STATUS "Build the project to generate the dependencies")
		message(STATUS "And then put COMPILE=1 to build the main project")
	endif()
elseif(${COMPILE} EQUAL 1)
	file(GLOB srcs "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/utils/*.cpp")
	add_executable (
		${PROJECT_NAME}
		${srcs}
	)
	if(APPLE)
		find_library (
			CURL_LIBRARY
			NAMES curl curllib libcurl_imp curllib_static libcurl libcurl_imp-d libcurl-d libcurl-d_imp
			HINTS "usr/lib"
			REQUIRED
		)
	else()
		find_library(CURL_LIBRARY
			NAMES curl curllib libcurl_imp curllib_static libcurl libcurl_imp-d libcurl-d libcurl-d_imp
			HINTS "${CMAKE_BINARY_DIR}/lib"
			REQUIRED
		)
		if(WIN32)
			find_library(PDCURSES_LIBRARY
				NAMES libpdcurses pdcurses curses
				HINTS "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/MinSizeRel/" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/Debug/" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/RelWithDebInfo/" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/Release/"
				REQUIRED
			)
		else()
			find_library(
				NCURSES_LIBRARY 
				NAMES libncurses ncurses
				HINTS "${CMAKE_BINARY_DIR}/lib"
				REQUIRED
			)
		endif()
	endif()
	if(WIN32)
		include_directories("include" "${CMAKE_BINARY_DIR}/include" "${nlohmann_json_SOURCE_DIR}/include/nlohmann" "${MarkIt_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project/" "${tabulate_SOURCE_DIR}/single_include/tabulate/")
		target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY} ${PDCURSES_LIBRARY} tabulate)
	elseif(APPLE)
		include_directories("usr/include" "include" "${CMAKE_BINARY_DIR}/include" "${CMAKE_SOURCE_DIR}/deps/tabulate/single_include/tabulate" "${nlohmann_json_SOURCE_DIR}/include/nlohmann")
		target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY} ${NCURSES_LIBRARY})
	else()
		include_directories("include" "${CMAKE_BINARY_DIR}/include" "${CMAKE_SOURCE_DIR}/deps/tabulate/single_include/tabulate" "${nlohmann_json_SOURCE_DIR}/include/nlohmann")
		target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY} ${NCURSES_LIBRARY} tabulate)
	endif()
	if(WIN32)
		message(STATUS "Curl Library: ${CURL_LIBRARY}")
		message(STATUS "PDCurses Library: ${PDCURSES_LIBRARY}")
	else()
		message(STATUS "Curl Library: ${CURL_LIBRARY}")
		message(STATUS "NCurses Library: ${NCURSES_LIBRARY}")
	endif()
	install(
		TARGETS ${PROJECT_NAME}
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
	unset(COMPILE CACHE)
endif()