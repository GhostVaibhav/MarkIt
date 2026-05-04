cmake_minimum_required(VERSION 3.15)
project(markit_patcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB srcs
    "src/*.cpp"
)

add_executable(${PROJECT_NAME} main.cpp ${srcs})

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall;-Wextra>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    "include"
    "${CMAKE_SOURCE_DIR}/deps/json/single_include/nlohmann"
    "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
    "${CMAKE_SOURCE_DIR}/include" # For picosha2 and utils
)

# Output the patcher to a dedicated tools directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools"
)
