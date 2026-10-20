cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS 
      ${CMAKE_CURRENT_LIST_DIR}/core
      ${CMAKE_CURRENT_LIST_DIR}/imu
      ${CMAKE_CURRENT_LIST_DIR}/motors
      ${CMAKE_CURRENT_LIST_DIR}/rx
   )

project(fribot)
