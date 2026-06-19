cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

# ─────────────────────────────────────────────────────────────────
# markit_testlib — shared production code compiled for tests.
# Excludes: main.cpp, panels/, app/, online/, update/
# (those pull in curses / curl / UI deps not needed here).
# ─────────────────────────────────────────────────────────────────
file(GLOB TEST_LIB_SRCS
    "${PROJECT_SOURCE_DIR}/src/models/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/files/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/db/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/managers/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/commands/*.cpp"
    # Logger lives under app/ but is needed by Todo::create()
    "${PROJECT_SOURCE_DIR}/src/app/Logger.cpp"
)

add_library(markit_testlib STATIC ${TEST_LIB_SRCS})

target_include_directories(markit_testlib PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/models"
    "${PROJECT_SOURCE_DIR}/include/config"
    "${PROJECT_SOURCE_DIR}/include/files"
    "${PROJECT_SOURCE_DIR}/include/db"
    "${PROJECT_SOURCE_DIR}/include/managers"
    "${PROJECT_SOURCE_DIR}/include/commands"
    "${PROJECT_SOURCE_DIR}/include/app"
    "${PROJECT_SOURCE_DIR}/include/enums"
    "${PROJECT_SOURCE_DIR}/include/utils"
    "${CMAKE_SOURCE_DIR}/deps/json/single_include/nlohmann"
    "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
)

target_link_libraries(markit_testlib PUBLIC SQLiteCpp spdlog::spdlog)

# ─────────────────────────────────────────────────────────────────
# Helper macro: create one gtest executable per test file
# ─────────────────────────────────────────────────────────────────
macro(markit_add_test TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        markit_testlib
        GTest::gtest_main
    )
    gtest_discover_tests(${TARGET_NAME})
endmacro()

# ─────────────────────────────────────────────────────────────────
# Individual test executables
# ─────────────────────────────────────────────────────────────────
markit_add_test(test_todo_db_manager   test_todo_db_manager.cpp)
markit_add_test(test_user_db_manager   test_user_db_manager.cpp)
markit_add_test(test_todo_manager      test_todo_manager.cpp)
markit_add_test(test_user_manager      test_user_manager.cpp)
markit_add_test(test_commands          test_commands.cpp)
markit_add_test(test_state_file        test_state_file.cpp)
markit_add_test(test_file_manager      test_file_manager.cpp)

# ─────────────────────────────────────────────────────────────────
# Patcher / Updater integration tests (separate subdirectory)
# ─────────────────────────────────────────────────────────────────
add_subdirectory(patcher_updater)
