cmake_minimum_required(VERSION 3.12)
project(Datasource)

find_package(PkgConfig)
pkg_check_modules(LIBMQTT REQUIRED libmosquitto)
pkg_check_modules(LIBMQTTPP REQUIRED libmosquittopp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -lmosquitto -lmosquittopp")
link_libraries(${LIBMQTT_LIBRARIES})
link_libraries(${LIBMQTTPP_LIBRARIES})
link_directories(${LIBMQTT_INCLUDE_DIRS})
link_directories(${LIBMQTTPP_INCLUDE_DIRS})

add_executable(Datasource Datasource.cpp Datasource.h)