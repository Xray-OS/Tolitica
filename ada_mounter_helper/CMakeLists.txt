cmake_minimum_required(VERSION 3.16)

project(ada_mounter_helper LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(ada_mounter_helper
  main.cpp
  ada_mounter_helper.h
  ada_mounter_helper.cpp
)
target_link_libraries(ada_mounter_helper Qt${QT_VERSION_MAJOR}::Core)

include(GNUInstallDirs)
install(TARGETS ada_mounter_helper
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
