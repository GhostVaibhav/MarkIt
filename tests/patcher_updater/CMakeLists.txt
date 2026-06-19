cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

# ─────────────────────────────────────────────────────────────────
# patcher_testlib — DiffEngine compiled for direct use in tests.
# Avoids subprocess overhead for patcher-level assertions.
# ─────────────────────────────────────────────────────────────────
add_library(patcher_testlib STATIC
    "${PROJECT_SOURCE_DIR}/tools/patcher/src/DiffEngine.cpp"
)

target_include_directories(patcher_testlib PUBLIC
    "${PROJECT_SOURCE_DIR}/tools/patcher/include"
    "${PROJECT_SOURCE_DIR}/deps/json/single_include/nlohmann"
    "${PROJECT_SOURCE_DIR}/deps/spdlog/include"
    "${PROJECT_SOURCE_DIR}/include"          # picosha2.h, utils/
)

target_link_libraries(patcher_testlib PUBLIC spdlog::spdlog)

# ─────────────────────────────────────────────────────────────────
# test_patcher — tests DiffEngine directly as a C++ library
# ─────────────────────────────────────────────────────────────────
add_executable(test_patcher test_patcher.cpp)

target_include_directories(test_patcher PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/deps/json/single_include/nlohmann"
    "${PROJECT_SOURCE_DIR}/include"
)

target_compile_definitions(test_patcher PRIVATE
    FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

target_link_libraries(test_patcher PRIVATE patcher_testlib GTest::gtest_main)

gtest_discover_tests(test_patcher)

# ─────────────────────────────────────────────────────────────────
# test_updater — end-to-end subprocess tests.
# Depends on both markit_patcher and markit_updater binaries.
# ─────────────────────────────────────────────────────────────────
add_executable(test_updater test_updater.cpp)

# Inject binary paths and fixtures path at compile time via defines.
# Generator expressions resolve to the actual output path at build time.
target_compile_definitions(test_updater PRIVATE
    PATCHER_BIN="$<TARGET_FILE:markit_patcher>"
    UPDATER_BIN="$<TARGET_FILE:markit_updater>"
    FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

target_include_directories(test_updater PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/deps/spdlog/include"
)

target_link_libraries(test_updater PRIVATE GTest::gtest_main spdlog::spdlog)

# Make sure both tool binaries are built before the test runs
add_dependencies(test_updater markit_patcher markit_updater)

gtest_discover_tests(test_updater)
