### Configs ###

cmake_minimum_required(VERSION 3.10)
project(phoenix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXEC_NAME "phoenix" CACHE STRING "Name of the executable")


### Dependencies ###

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(CLI11 REQUIRED CLI11)
pkg_check_modules(SPDLOG REQUIRED spdlog)

### Executable ###

set(SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/logger.cpp
)

add_executable("${EXEC_NAME}" ${SRC_FILES})
target_include_directories("${EXEC_NAME}" PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GTKMM_INCLUDE_DIRS}
        ${CLI11_INCLUDE_DIRS}
        ${SPDLOG_INCLUDE_DIRS}
)

target_compile_options("${EXEC_NAME}" PRIVATE
        ${GTKMM_CFLAGS_OTHER}
        ${CLI11_CFLAGS_OTHER}
        ${SPDLOG_CFLAGS_OTHER}
)

target_link_libraries("${EXEC_NAME}" PRIVATE
        ${GTKMM_LIBRARIES}
        ${CLI11_LIBRARIES}
        ${SPDLOG_LIBRARIES}
)
