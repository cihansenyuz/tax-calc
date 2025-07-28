cmake_minimum_required(VERSION 3.5)

project(tax_calc)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_PREFIX_PATH "/home/cihan/Qt/6.9.1/gcc_64/lib/cmake")
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        src/main.cpp

        src/network/networkcore.cpp
        src/network/getrequest.cpp
        src/network/evdsfetcher.cpp
        src/assetmanager.cpp

        inc/network/networkcore.h
        inc/network/getrequest.h
        inc/network/httpmanager.h
        inc/network/evdsfetcher.h
        inc/assetmanager.hpp

        gui/mainwindow.ui
        gui/mainwindow.cpp
        gui/mainwindow.hpp

        gui/createdialog.ui
        gui/createdialog.cpp
        gui/createdialog.hpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network)
