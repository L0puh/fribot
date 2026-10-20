cmake_minimum_required(VERSION 3.16)

project(workspace LANGUAGES NONE)

include(ExternalProject)

ExternalProject_Add(fribot
    SOURCE_DIR      ${CMAKE_CURRENT_LIST_DIR}/fribot
    BINARY_DIR      ${CMAKE_CURRENT_LIST_DIR}/fribot/build
    CMAKE_ARGS      ${COMMON_CMAKE_ARGS}
    BUILD_ALWAYS    ON
    INSTALL_COMMAND ""
)

ExternalProject_Add(controller
    SOURCE_DIR      ${CMAKE_CURRENT_LIST_DIR}/controller
    BINARY_DIR      ${CMAKE_CURRENT_LIST_DIR}/controller/build
    CMAKE_ARGS      ${COMMON_CMAKE_ARGS}
    BUILD_ALWAYS    ON
    INSTALL_COMMAND ""
)

add_custom_target(firmware
    DEPENDS fribot controller
)
