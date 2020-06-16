cmake_minimum_required(VERSION 3.0.0)

project(deepin-image-viewer VERSION 0.1 LANGUAGES CXX)

#"option"用来定义宏，"ON"表示打开，"OFF"表示关闭
option (LITE_DIV "Use tutorial provided math implementation" ON)
add_definitions( -DLITE_DIV )

set(TARGET_NAME deepin-image-viewer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#ADD_DEFINITIONS( -LITE_DIV)
execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DMR_DEBUG on)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Sources files
#cmakedefine USE_MYMATH

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE FORMS "*.ui")
file(GLOB_RECURSE RESOURCES "*.qrc")

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/utils
    ${PROJECT_SOURCE_DIR}/controller
    ${PROJECT_SOURCE_DIR}/dirwatcher
    ${PROJECT_SOURCE_DIR}/widgets)

find_package(PkgConfig REQUIRED)

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Sql)
find_package(Qt5DBus)
find_package(Qt5Concurrent)
find_package(Qt5Svg)
find_package(Qt5X11Extras)
find_package(Qt5PrintSupport)
find_package(Qt5OpenGL)
find_package(dtkWidget)

pkg_check_modules(3rd_lib REQUIRED
    dtkwidget
    dtkcore
    gio-qt
    udisks2-qt5
    gio-unix-2.0
#    freeimage
        )


add_executable( ${TARGET_NAME} ${SOURCES} ${HEADERS} ${RESOURCES} ${PROJECT_INCLUDE})

target_include_directories(${TARGET_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} ${PROJECT_INCLUDE} ${GLOB_RECURSE})
target_link_libraries(${TARGET_NAME} ${3rd_lib_LIBRARIES} )


target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Svg
    Qt5::DBus
    Qt5::Concurrent
    Qt5::X11Extras
    Qt5::PrintSupport
    Qt5::OpenGL
    Qt5::WidgetsPrivate
    dtkwidget
    dtkcore
    X11
    Xext
    exif
#    gio-unix-2.0
    gio-qt
    udisks2-qt5
    freeimage
    pthread
    )
install(TARGETS ${TARGET_NAME} DESTINATION bin)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})
configure_file(${PROJECT_SOURCE_DIR}/config.h.in  @ONLY)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)


#install
set(PREFIX  /usr)
set(BINDIR  ${PREFIX}/bin)
set(APPSHAREDIR  ${PREFIX}/share/deepin-image-viewer)
set(MANDIR  ${PREFIX}/share/dman/deepin-image-viewer)
set(MANICONDIR  ${PREFIX}/share/icons/hicolor/scalable/apps)
set(APPICONDIR  ${PREFIX}/share/icons/hicolor/scalable/apps)

#target
install(TARGETS ${CMD_NAME} DESTINATION bin)
#desktop
install(FILES ${PROJECT_SOURCE_DIR}/viewer/deepin-image-viewer.desktop
    DESTINATION usr/share/applications)
#icons
install(FILES ${CMAKE_SOURCE_DIR}/viewer/assets/images/*
    DESTINATION usr/share/deepin-image-viewer/icons)
#manual
install(FILES ${PROJECT_SOURCE_DIR}/viewer/docs/doc/*
    DESTINATION usr/share/dman/deepin-image-viewer)
#manual_icon
install(FILES ${PROJECT_SOURCE_DIR}/viewer/docs/doc/common/deepin-image-viewer.svg
    DESTINATION usr/share/icons/hicolor/scalable/apps)
#app_icon
install(FILES ${PROJECT_SOURCE_DIR}/viewer/assets/images/logo/deepin-image-viewer.svg
    DESTINATION usr/share/icons/hicolor/scalable/apps)
#dbus_service
install(FILES ${CMAKE_SOURCE_DIR}/viewer/com.deepin.ImageViewer.service
    DESTINATION usr/share/dbus-1/services)
#translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/viewer/translations
    DESTINATION share/${CMD_NAME}
    FILES_MATCHING PATTERN "*.qm")
