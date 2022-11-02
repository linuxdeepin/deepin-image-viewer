cmake_minimum_required(VERSION 3.0.0)

project(deepin-image-viewer VERSION 1.0.0)

##"option"用来定义宏，"ON"表示打开，"OFF"表示关闭
#option (LITE_DIV "Use tutorial provided math implementation" ON)
##使用UnionImage
#option (USE_UNIONIMAGE "Use UNIONIMAGE" ON)
add_definitions( -DLITE_DIV )
add_definitions( -DUSE_UNIONIMAGE )
add_definitions( -DCMAKE_BUILD )
# 加速编译优化参数
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-need -fPIE")
    SET(CMAKE_C_FLAGS "$ENV{CFLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-need -fPIE")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pie")
else()
    SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O3 -Wl,--as-need -fPIE")
    SET(CMAKE_C_FLAGS "$ENV{CFLAGS} -O3  -Wl,--as-need -fPIE")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pie")
endif()

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "sw_64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mieee")
endif()

set(TARGET_NAME deepin-image-viewer)
set(CMD_NAME deepin-image-viewer)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE  SOURCES  "src/*.cpp" "src/*/*.cpp")
file(GLOB_RECURSE SOURCESC "src/*.c")
file(GLOB_RECURSE HEADERS "src/module/modulepanel.h" "src/*/*.h")
file(GLOB_RECURSE FORMS "src/*.ui")
file(GLOB_RECURSE RESOURCES "*.qrc")

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories("../libimage-viewer/")

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

set(PROJECT_INCLUDE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/utils
    ${PROJECT_SOURCE_DIR}/src/controller
    ${PROJECT_SOURCE_DIR}/src/dirwatcher
    ${PROJECT_SOURCE_DIR}/src/widgets
    ${PROJECT_SOURCE_DIR}/src/accessibility
    ${PROJECT_SOURCE_DIR}/src/third-party/accessibility
    ${PROJECT_SOURCE_DIR}/src/module
    )

set(QtModule Core Gui Widgets LinguistTools DBus)

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget
    dtkcore
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

add_executable( ${PROJECT_NAME} ${SOURCES} ${SOURCESC} ${HEADERS} ${RESOURCES} ${QM})

target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} ${PROJECT_INCLUDE} )
target_link_libraries(${PROJECT_NAME}  ${3rd_lib_LIBRARIES} pthread imageviewer)
#将工程与这些模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option (NOUSE_TEST " NO Use TESTS" ON)
add_definitions( -DNOUSE_TEST )

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
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets/deepin-image-viewer
    DESTINATION /usr/share/deepin-manual/manual-assets/application/
    FILES_MATCHING PATTERN "*")
#manual_icon
install(FILES ${PROJECT_SOURCE_DIR}/assets/doc/common/deepin-image-viewer.svg
    DESTINATION /usr/share/icons/hicolor/scalable/apps)
#app_icon
install(FILES ${PROJECT_SOURCE_DIR}/assets/images/logo/deepin-image-viewer.svg
    DESTINATION /usr/share/icons/hicolor/scalable/apps)
#dbus_service
install(FILES ${PROJECT_SOURCE_DIR}/com.deepin.ImageViewer.service
    DESTINATION /usr/share/dbus-1/services)
#translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${APPSHAREDIR}
    FILES_MATCHING PATTERN "*.qm")



