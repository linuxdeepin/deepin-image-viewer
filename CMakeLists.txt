cmake_minimum_required(VERSION 3.1)

project(deepin-image-viewer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLEOCR "Enable OCR" OFF)

include_directories(${PROJECT_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

find_package(Qt5 REQUIRED COMPONENTS
  Core Gui Quick QuickCompiler
  DBus Concurrent Svg PrintSupport LinguistTools)
find_package(Qt5Gui)
find_package(DtkDeclarative REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(PkgConfig REQUIRED)

# Application
add_subdirectory(src)

#add_subdirectory(qimage-plugins)

# Unit Tests
#add_subdirectory(tests)
