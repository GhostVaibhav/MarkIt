cmake_minimum_required (VERSION 3.8)

include(ExternalProject)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE ${CMAKE_COMMAND})

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()


if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR “${BoldRed}In-source build detected!${ColourReset}”)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "MinSizeRel")
endif()

project ("MarkIt")

if((DEFINED INFOEX) AND (INFOEX EQUAL 1))
	set(CMAKE_C_FLAGS "-DHAVE_NO_INFOEX")
	set(CMAKE_CXX_FLAGS "-DHAVE_NO_INFOEX")
else()
	set(INFOEX 0)
endif()

add_subdirectory(deps/tabulate)

if(DEFINED COMPILE)
	message(STATUS "${Blue}COMPILE = ${COMPILE}${ColourReset}")
else()
	message(STATUS "${BoldGreen}COMPILE not supplied${ColourReset}")
	message(STATUS "${BoldGreen}Defaulting to COMPILE = 1${ColourReset}")
	set(COMPILE 1)
endif()

if(${COMPILE} EQUAL 0)
	if(WIN32)
		ExternalProject_Add(
			pdcurses_project
			URL "https://github.com/GhostVaibhav/PDCurses/archive/refs/tags/4.1.zip"
			INSTALL_COMMAND ""
			CMAKE_ARGS
			-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
			-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
		)
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
	else()
		if(APPLE)
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
	message(STATUS "${Green}Build the project to generate the dependencies${ColourReset}")
	message(STATUS "${Cyan}And then compile the project without any args${ColourReset}")
	message(STATUS "${Cyan}for building the main project${ColourReset}")
else()
	file(GLOB srcs "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/utils/*.cpp")
	add_executable (${PROJECT_NAME} ${srcs})
	find_library (
		CURL_LIBRARY
		NAMES curl curllib libcurl_imp curllib_static libcurl libcurl_imp-d libcurl-d libcurl-d_imp
		HINTS "${CMAKE_BINARY_DIR}/lib" "usr/lib"
		REQUIRED
	)
	if(WIN32)
		find_library (
			PDCURSES_LIBRARY
			NAMES libpdcurses pdcurses curses
			HINTS "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/${CMAKE_BUILD_TYPE}"
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
	if(WIN32)
		include_directories("include" "${CMAKE_BINARY_DIR}/include" "${nlohmann_json_SOURCE_DIR}/include/nlohmann" "${MarkIt_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project/" "${tabulate_SOURCE_DIR}/single_include/tabulate/")
		target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY} ${PDCURSES_LIBRARY} tabulate)
	elseif(APPLE)
		include_directories("usr/include" "include" "${CMAKE_BINARY_DIR}/include" "${CMAKE_SOURCE_DIR}/deps/tabulate/single_include/tabulate" "${nlohmann_json_SOURCE_DIR}/include/nlohmann")
		target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY} ${NCURSES_LIBRARY})
	else()
		include_directories("include" "${CMAKE_BINARY_DIR}/include" "${CMAKE_SOURCE_DIR}/deps/tabulate/single_include/tabulate" "${nlohmann_json_SOURCE_DIR}/include/nlohmann")
		target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY} ${NCURSES_LIBRARY})
	endif()
	# if(MSVC)
	# 	target_compile_options(${PROJECT_NAME} PRIVATE /MT)
	# 	target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
	# elseif(CMAKE_CXX_COMPILER_ID EQUAL GNU OR CMAKE_CXX_COMPILER_ID EQUAL Clang)
	# 	set(CMAKE_EXE_LINKER_FLAGS "-static")
	# else()
	# 	target_link_libraries(${PROJECT_NAME} "-static")
	# endif()
	unset(COMPILE CACHE)
	message(STATUS "${}Project Name: ${PROJECT_NAME}")
	message(STATUS "${}Build Type: ${CMAKE_BUILD_TYPE}")
	message(STATUS "${}Build Directory: ${CMAKE_BINARY_DIR}")
	message(STATUS "${}Source Directory: ${PROJECT_SOURCE_DIR}")
	message(STATUS "${}COMPILE = ${COMPILE}")
	message(STATUS "${}INFOEX = ${INFOEX}")
	message(STATUS "${}Curl library path: ${CURL_LIBRARY}")
	if(WIN32)
		message(STATUS "${}PDCurses library path: ${PDCURSES_LIBRARY}")
	else()
		message(STATUS "${}Ncurses library path: ${NCURSES_LIBRARY}")
	endif()
endif()