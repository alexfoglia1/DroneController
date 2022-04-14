cmake_minimum_required(VERSION 3.10)
project(DroneController CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(
DroneController
${CMAKE_SOURCE_DIR}/modules/gui/DroneControllerWindow.h
${CMAKE_SOURCE_DIR}/modules/gui/DroneControllerWindow.cpp
${CMAKE_SOURCE_DIR}/modules/gui/widgets/ControllerFrame.h
${CMAKE_SOURCE_DIR}/modules/gui/widgets/ControllerFrame.cpp
${CMAKE_SOURCE_DIR}/modules/gui/widgets/ControllerMenu.h
${CMAKE_SOURCE_DIR}/modules/gui/widgets/ControllerMenu.cpp
${CMAKE_SOURCE_DIR}/modules/gui/widgets/JoystickFrame.h
${CMAKE_SOURCE_DIR}/modules/gui/widgets/JoystickFrame.cpp

${CMAKE_SOURCE_DIR}/modules/joystick/joystick.h
${CMAKE_SOURCE_DIR}/modules/joystick/joystick.cpp

${CMAKE_SOURCE_DIR}/modules/radio/proto.h


${CMAKE_SOURCE_DIR}/modules/app/app.h
${CMAKE_SOURCE_DIR}/modules/app/app.cpp
)

target_include_directories(DroneController PRIVATE ${CMAKE_SOURCE_DIR}/modules)

find_package(Threads REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets Network OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
target_link_libraries(DroneController ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${SDL2_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Network stdc++fs pthread)

install(TARGETS DroneController DESTINATION bin)
