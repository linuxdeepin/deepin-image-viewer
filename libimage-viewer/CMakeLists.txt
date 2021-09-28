#定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(libimage-viewer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#Qt需要的包
set(QtModule Core Gui Widgets Svg DBus Concurrent PrintSupport LinguistTools)

set(TARGET_NAME image-viewer)
set(CMD_NAME image-viewer)

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

#设置输出目录
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../deepin-album/lib)

#包含目录
include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions( -DLITE_DIV )
add_definitions( -DUSE_UNIONIMAGE )
add_definitions( -DCMAKE_BUILD )

#库目录
aux_source_directory(image-viewer allSources)

find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget
    dtkcore
    )

#需要打开的头文件
FILE (GLOB allHeaders "*.h" "*/*.h" "*/*/*.h")
#需要打开的代码文件
FILE (GLOB allSources
    "*.cpp"                                   "*.c"
    "*/*.cpp"                                 "*/*.c"
    "*/*/*.cpp"                              "*/*/*.c"
    )
file(GLOB_RECURSE RESOURCES "*.qrc")

#------------------添加第三方库begins--------------------------#
#使用第三方库需要用到的一个包

set(INC_DIR /usr/include/)
set(LINK_DIR /usr/lib/)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})

link_libraries(freeimage)

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

file(GLOB TS LIST_DIRECTORIES false translations/libimage-viewer*.ts)

set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)

qt5_create_translation(QM ${allSources}  ${GLOB})

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(QM ${allSources} ${TS})
endif ()

# 设置不删除生成的文件夹内容文件
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#编译为库
add_library(image-viewer SHARED ${allHeaders} ${allSources} ${RESOURCES} ${QM})
#使用库
qt5_use_modules(image-viewer ${QtModule})

#将库安装到指定位置
set(PREFIX /usr)
set(image-viewer ${CMAKE_INSTALL_LIBDIR})


set_target_properties(${CMD_NAME} PROPERTIES VERSION 0.1.0 SOVERSION 0.1)
target_include_directories(${CMD_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS}  )
target_link_libraries(image-viewer ${3rd_lib_LIBRARIES} freeimage )

include(GNUInstallDirs)
configure_file(libimage-viewer.pc.in ${PROJECT_BINARY_DIR}/libimage-viewer.pc @ONLY)

install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES
    imageengine.h
    imageviewer.h
    DESTINATION include/libimage-viewer)

install(FILES ${PROJECT_BINARY_DIR}/libimage-viewer.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
#translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${PREFIX}/share/libimage-viewer
    FILES_MATCHING PATTERN "*.qm")
