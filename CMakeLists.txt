cmake_minimum_required(VERSION 3.21)

project(NokiaFBusSniffer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets SerialPort)

qt_add_executable(NokiaFBusSniffer
    main.cpp
    MainWindow.h
    MainWindow.cpp
    SerialManager.h
    SerialManager.cpp
    LogFormatter.h
    LogFormatter.cpp
    SettingsManager.h
    SettingsManager.cpp
)

target_link_libraries(NokiaFBusSniffer
    PRIVATE
        Qt6::Widgets
        Qt6::SerialPort
)

if (MSVC)
    target_compile_options(NokiaFBusSniffer PRIVATE /W4 /permissive-)
else()
    target_compile_options(NokiaFBusSniffer PRIVATE -Wall -Wextra -Wpedantic)
endif()
