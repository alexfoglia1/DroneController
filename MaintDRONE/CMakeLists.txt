cmake_minimum_required(VERSION 3.6)

project(MaintDRONE)

add_executable(MaintDRONE
${CMAKE_SOURCE_DIR}/../FlightController/MAINT.h
${CMAKE_SOURCE_DIR}/../FlightController/MAINT.cpp

${CMAKE_SOURCE_DIR}/main.cpp)

target_include_directories(MaintDRONE PUBLIC ${CMAKE_SOURCE_DIR}/../FlightController)