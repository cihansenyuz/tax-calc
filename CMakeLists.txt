cmake_minimum_required(VERSION 3.5)

project(tax_calc)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_PREFIX_PATH "/home/cihan/Qt/6.9.1/gcc_64/lib/cmake") # for ubuntu
# set(CMAKE_PREFIX_PATH "C:\\Qt\\6.9.0\\llvm-mingw_64\\lib\\cmake") # for windows
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql)

set(PROJECT_SOURCES
        src/main.cpp
        src/logger.cpp

        src/network/networkcore.cpp
        src/network/getrequest.cpp
        src/network/evdsfetcher.cpp
        src/network/yahoofinancefetcher.cpp
        src/database/transactiondatabase.cpp
        src/transactionmanager.cpp

        inc/logger.hpp
        inc/network/networkcore.hpp
        inc/network/getrequest.hpp
        inc/network/httpmanager.hpp
        inc/network/evdsfetcher.hpp
        inc/network/yahoofinancefetcher.hpp
        inc/database/transactiondatabase.hpp
        inc/transactionmanager.hpp
        inc/calculator.hpp

        gui/mainwindow.ui
        gui/mainwindow.cpp
        gui/mainwindow.hpp
        gui/transactiontable.hpp
        gui/createdialog.ui
        gui/createdialog.cpp
        gui/createdialog.hpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network Qt6::Sql)
