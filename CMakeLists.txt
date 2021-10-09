cmake_minimum_required(VERSION 3.16.0)

project(SwissTournament VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

set(UI MainWindow.ui)
set(SOURCE main.cpp MainWindow.cpp match.cpp player.cpp)
set(HEADER MainWindow.hpp match.hpp player.hpp)

add_executable(${PROJECT_NAME} ${UI} ${SOURCE} ${HEADER})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

