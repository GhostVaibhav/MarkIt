cmake_minimum_required (VERSION 3.8)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "Final install location")
set(CMAKE_BUILD_TYPE_INIT "MinSizeRel")
set(CMAKE_CONFIGURATION_TYPES "MinSizeRel")

project ("MarkIt")

include_directories("include")

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "src/main.cpp" "include/sha256.cpp")

if(WIN32)
	ExternalProject_Add(
	 	pdcurses_project
	 	URL "https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.zip"
		CMAKE_ARGS
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		BUILD_COMMAND ""
		CONFIGURE_COMMAND ""
		INSTALL_COMMAND ""
	)
	ExternalProject_Add_Step(
		pdcurses_project
		copy
		if(WIN32)
			COMMAND copy "${CMAKE_SOURCE_DIR}/include/CMakeLists.txt" "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project"
			# copy <INSTALL_PREFIX>/../include/CMakeLists.txt <INSTALL_PREFIX>/pdcurses_project-prefix/src/pdcurses_project /Y
		elseif(UNIX)
			COMMAND "ls"
		endif()
		COMMAND 
		# "file(COPY "${CMAKE_SOURCE_DIR}/include/CMakeLists.txt" DESTINATION "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project")"
	)
	
	ExternalProject_Add(
		curl_project
		URL "https://github.com/curl/curl/releases/download/curl-7_77_0/curl-7.77.0.zip"
		CMAKE_ARGS
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
			-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
			-DBUILD_CURL_EXE=OFF
			-DBUILD_SHARED_LIBS=ON
			-DBUILD_TESTING=OFF
			-DCMAKE_USE_SCHANNEL=ON
			-DUSE_WIN32_LDAP=ON
			-DENABLE_INET_PTON=OFF
			-DENABLE_THREADED_RESOLVER=OFF
			-DUSE_LIBIDN2=OFF
	)
	
	add_dependencies(${PROJECT_NAME} curl_project pdcurses_project)
	find_library(pdcurses_LIBRARY "pdcurses" PATHS "${CMAKE_BINARY_DIR}/pdcurses_project-prefix/src/pdcurses_project-build/${CMAKE_BUILD_TYPE}/")
	if(MSVC)
		find_library(curl_LIBRARY "curl_imp" PATHS "${CMAKE_BINARY_DIR}/lib/")
	else()
		find_library(curl_LIBRARY "curl" PATHS "${CMAKE_BINARY_DIR}/lib/")
	endif()
	target_link_libraries(${PROJECT_NAME} STATIC CURL pdcurses)
else()
target_link_libraries(${PROJECT_NAME} curl ncurses)
endif()