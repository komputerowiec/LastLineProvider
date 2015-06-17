#INCLUDE(CPack)
cmake_minimum_required (VERSION 2.6)

project (LastLineProvider)

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

configure_file ("${PROJECT_SOURCE_DIR}/configuration.h.in" "${PROJECT_BINARY_DIR}/configuration.h")

include_directories("${PROJECT_BINARY_DIR}")

add_executable(lastlineprovider main.c)

install (TARGETS lastlineprovider DESTINATION bin)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

