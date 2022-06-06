cmake_minimum_required(VERSION 3.10)
project(DroneController CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (WIN32)
    set(SDL2_INCLUDE_DIRS "C:/SDL2-devel-2.0.20-VC/SDL2-2.0.20/include")

    # Support both 32 and 64 bit builds
    if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
        set(SDL2_LIBRARIES "C:/SDL2-devel-2.0.20-VC/SDL2-2.0.20/lib/x64/SDL2.lib;C:/SDL2-devel-2.0.20-VC/SDL2-2.0.20/lib/x64/SDL2main.lib")
    else ()
        set(SDL2_LIBRARIES "C:/SDL2-devel-2.0.20-VC/SDL2-2.0.20/lib/x86/SDL2.lib;C:/SDL2-devel-2.0.20-VC/SDL2-2.0.20/lib/x86/SDL2main.lib")
    endif ()

    string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
endif()

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
${CMAKE_SOURCE_DIR}/modules/gui/widgets/DroneFrame.h
${CMAKE_SOURCE_DIR}/modules/gui/widgets/DroneFrame.cpp

${CMAKE_SOURCE_DIR}/modules/joystick/joystick.h
${CMAKE_SOURCE_DIR}/modules/joystick/joystick.cpp

${CMAKE_SOURCE_DIR}/modules/radio/proto.h
${CMAKE_SOURCE_DIR}/modules/radio/radio.h
${CMAKE_SOURCE_DIR}/modules/radio/radio.cpp

${CMAKE_SOURCE_DIR}/modules/settings/rapidxml.hpp
${CMAKE_SOURCE_DIR}/modules/settings/rapidxml_iterators.hpp
${CMAKE_SOURCE_DIR}/modules/settings/rapidxml_print.hpp
${CMAKE_SOURCE_DIR}/modules/settings/rapidxml_utils.hpp
${CMAKE_SOURCE_DIR}/modules/settings/settings.h
${CMAKE_SOURCE_DIR}/modules/settings/settings.cpp


${CMAKE_SOURCE_DIR}/modules/app/app.h
${CMAKE_SOURCE_DIR}/modules/app/app.cpp
)

target_include_directories(DroneController PRIVATE ${CMAKE_SOURCE_DIR}/modules)

find_package(Threads REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets Network OpenGL SerialPort REQUIRED)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(SDL2 REQUIRED)
endif()

include_directories(${SDL2_INCLUDE_DIRS})
if (WIN32)
    target_link_libraries(DroneController ${SDL2_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Network Qt5::SerialPort)
else()
    target_link_libraries(DroneController ${SDL2_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Network Qt5::SerialPort pthread)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    install(TARGETS DroneController DESTINATION bin)
endif()

add_custom_command(TARGET DroneController POST_BUILD
    COMMAND cp ${CMAKE_SOURCE_DIR}/modules/settings/DefaultSettings.xml ${CMAKE_CURRENT_BINARY_DIR}/Settings.xml
)
