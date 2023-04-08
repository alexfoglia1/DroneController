cmake_minimum_required(VERSION 3.6)

project(MaintDRONE)


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(MaintDRONE
${CMAKE_SOURCE_DIR}/../FlightController/MAINT.h
${CMAKE_SOURCE_DIR}/../FlightController/MAINT.cpp

${CMAKE_SOURCE_DIR}/MaintDRONE.ui
${CMAKE_SOURCE_DIR}/PlotWidget.h
${CMAKE_SOURCE_DIR}/PlotWidget.cpp
${CMAKE_SOURCE_DIR}/main.cpp)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

target_include_directories(MaintDRONE PUBLIC ${CMAKE_SOURCE_DIR}/../FlightController)
target_link_libraries(MaintDRONE Qt5::Core Qt5::Gui Qt5::Widgets)
