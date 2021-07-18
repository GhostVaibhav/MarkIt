cmake_minimum_required (VERSION 3.8)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE_BUILD_TYPE_INIT "MinSizeRel")
set(CMAKE_CONFIGURATION_TYPES "MinSizeRel")

project ("Todo")

include_directories("include")

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "main.cpp" "include/sha256.cpp")

if(WIN32)
	link_directories("lib")
	ExternalProject_Add(
	 	pdcurses
	 	URL "https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.zip"
	 	# CMAKE_ARGS
	 	# 	-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	 	# 	-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	 	# 	-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
	 	# 	-DPDCDEBUG=OFF
	 	# 	-DPDC_BUILD_SHARED=OFF
	 	# 	-DPDC_SDL2_BUILD=OFF
	 	# 	-DPDC_SDL2_DEPS_BUILD=OFF
	 	# 	-DPDC_DOSVT_BUILD=OFF
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)


	# if(MSVC)
	# 	ExternalProject_Add_Step(pdcurses "Installing" COMMAND ${CMAKE_MAKE_PROGRAM} -f "${CMAKE_BINARY_DIR}/pdcurses-prefix/src/pdcurses/wincon/Makefile.vc" DLL=N)
	# elseif(MINGW)
	# 	ExternalProject_Add_Step(pdcurses "Installing" COMMAND ${CMAKE_MAKE_PROGRAM} -f "${CMAKE_BINARY_DIR}/pdcurses-prefix/src/pdcurses/wincon/Makefile" DLL=N INFOEX=N)
	# elseif(CYGWIN)
	# elseif(MSYS)
	# elseif(BORLAND)
	# elseif(WATCOM)
	# endif()

	
	ExternalProject_Add(
		curl
		URL "https://github.com/curl/curl/releases/download/curl-7_77_0/curl-7.77.0.zip"
		CMAKE_ARGS
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
			-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
			-DBUILD_CURL_EXE=OFF
			-DBUILD_SHARED_LIBS=OFF
			-DBUILD_TESTING=OFF
			-DCMAKE_USE_SCHANNEL=ON
			-DUSE_WIN32_LDAP=ON
			-DENABLE_INET_PTON=OFF
			-DENABLE_THREADED_RESOLVER=OFF
			-DUSE_LIBIDN2=OFF
	)
	
	add_dependencies(${PROJECT_NAME} curl pdcurses)
	find_library(curl_lib libcurl HINTS "${CMAKE_BINARY_DIR}/build/lib")
	find_library(pdc_lib pdcursesstatic HINTS "${CMAKE_BINARY_DIR}/build/pdcurses_mod-prefix/src/	pdcurses_mod-build/wincon/MinSizeRel")
endif()
target_link_libraries(${PROJECT_NAME} curl ncurses)