CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

project(DroneJoystick)

add_executable(drone-joy
${CMAKE_SOURCE_DIR}/modules/joystick/joystick.h
${CMAKE_SOURCE_DIR}/modules/joystick/joystick.cpp

${CMAKE_SOURCE_DIR}/modules/main/main.cpp
)


