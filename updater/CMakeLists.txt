cmake_minimum_required(VERSION 3.10)

project("markit_updater" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)

# No external dependencies — uses only standard library
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall;-Wextra>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_include_directories(${PROJECT_NAME} PRIVATE 
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/deps/json/single_include/nlohmann
  ${CMAKE_SOURCE_DIR}/include # For picosha2
)

# Output the updater executable to the same directory as the main MarkIt executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT Application)
