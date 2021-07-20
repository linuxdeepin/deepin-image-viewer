#定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(libimage-viewer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#Qt需要的包
set(QtModule Core Gui Widgets Svg Concurrent)

set(TARGET_NAME image-viewer)
set(CMD_NAME image-viewer)

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

#设置输出目录
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../deepin-album/lib)

#包含目录
include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#库目录
aux_source_directory(image-viewer allSources)

find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget
    dtkcore
    )

#需要打开的头文件
FILE (GLOB allHeaders "*.h")

#需要打开的代码文件
FILE (GLOB allSources
    "*.cpp"                                   "*.c"
    )

#------------------添加第三方库begins--------------------------#
#使用第三方库需要用到的一个包

set(INC_DIR /usr/include/)
set(LINK_DIR /usr/lib/)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})


link_libraries(freeimage)



#编译为库
add_library(image-viewer SHARED ${allHeaders} ${allSources})

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#使用库
qt5_use_modules(image-viewer ${QtModule})

#将库安装到指定位置
set(PREFIX /usr)
set(image-viewer ${PREFIX}/lib)

install(TARGETS image-viewer LIBRARY DESTINATION ${image-viewer})

execute_process(
COMMAND sudo ln -s ${image-viewer}/libimage-viewer.so ${image-viewer}/libimage-viewer.so.1
COMMAND sudo ln -s ${image-viewer}/libimage-viewer.so ${image-viewer}/libimage-viewer.so.1.0
COMMAND sudo ln -s ${image-viewer}/libimage-viewer.so ${image-viewer}/libimage-viewer.so.1.0.0
)

target_include_directories(${CMD_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS}  )
target_link_libraries(image-viewer ${3rd_lib_LIBRARIES} freeimage)
