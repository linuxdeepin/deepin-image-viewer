cmake_minimum_required(VERSION 3.0.0)

project(deepin-image-viewer)

##"option"用来定义宏，"ON"表示打开，"OFF"表示关闭
#option (LITE_DIV "Use tutorial provided math implementation" ON)
##使用UnionImage
#option (USE_UNIONIMAGE "Use UNIONIMAGE" ON)
add_definitions( -DLITE_DIV )
add_definitions( -DUSE_UNIONIMAGE )

set(TARGET_NAME deepin-image-viewer)
set(CMD_NAME deepin-image-viewer)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE  SOURCES  "*.cpp")
file(GLOB_RECURSE SOURCESC "*.c")
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE FORMS "*.ui")
file(GLOB_RECURSE RESOURCES "*.qrc")

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/utils
    ${PROJECT_SOURCE_DIR}/controller
    ${PROJECT_SOURCE_DIR}/dirwatcher
    ${PROJECT_SOURCE_DIR}/widgets
    ${PROJECT_SOURCE_DIR}/accessibility
    ${PROJECT_SOURCE_DIR}/third-party/accessibility)

set(QtModule Core Gui Widgets DBus Sql PrintSupport Svg Concurrent LinguistTools )

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget
    dtkcore
    gio-qt
    udisks2-qt5
    libexif
    )

## translations

file(GLOB TS LIST_DIRECTORIES false translations/${CMD_NAME}*.ts)

set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)

qt5_create_translation(QM ${SOURCES}  ${GLOB})

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(QM ${SOURCES} ${TS})
endif ()

# 设置不删除生成的文件夹内容文件
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

add_executable( ${PROJECT_NAME} ${SOURCES} ${SOURCESC} ${HEADERS} ${RESOURCES} ${PROJECT_INCLUDE} ${QM})

target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} ${PROJECT_INCLUDE} )
target_link_libraries(${PROJECT_NAME} Qt5::WidgetsPrivate ${3rd_lib_LIBRARIES} pthread freeimage)

#将工程与这些模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#install
set(PREFIX  /usr)
set(BINDIR  ${PREFIX}/bin)
set(APPSHAREDIR  ${PREFIX}/share/deepin-image-viewer)
set(MANDIR  ${PREFIX}/share/dman/deepin-image-viewer)
set(MANICONDIR  ${PREFIX}/share/icons/hicolor/scalable/apps)
set(APPICONDIR  ${PREFIX}/share/icons/hicolor/scalable/apps)

install(TARGETS ${TARGET_NAME} DESTINATION ${BINDIR})
#desktop
install(FILES ${PROJECT_SOURCE_DIR}/deepin-image-viewer.desktop
    DESTINATION /usr/share/applications)
#icons
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets/images/
    DESTINATION /usr/share/deepin-image-viewer/icons
    FILES_MATCHING PATTERN "*")
#manual
install(DIRECTORY ${PROJECT_SOURCE_DIR}/docs/doc/
    DESTINATION /usr/share/dman/deepin-image-viewer
    FILES_MATCHING PATTERN "*")
#manual_icon
install(FILES ${PROJECT_SOURCE_DIR}/docs/doc/common/deepin-image-viewer.svg
    DESTINATION /usr/share/icons/hicolor/scalable/apps)
#app_icon
install(FILES ${PROJECT_SOURCE_DIR}/assets/images/logo/deepin-image-viewer.svg
    DESTINATION /usr/share/icons/hicolor/scalable/apps)
#dbus_service
install(FILES ${CMAKE_SOURCE_DIR}/viewer/com.deepin.ImageViewer.service
    DESTINATION /usr/share/dbus-1/services)
#translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${APPSHAREDIR}
    FILES_MATCHING PATTERN "*.qm")
