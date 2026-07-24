# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

if (NOT DEFINED VERSION)
    set(VERSION "1.0.0")
endif ()

# 开启 Qt MOC/UIC/RCC，与 src/CMakeLists.txt 保持一致
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

ADD_COMPILE_OPTIONS(-fno-access-control)

set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")

if(CMAKE_SAFETYTEST STREQUAL "")
    set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
endif()

# 安全编译参数（统一管理，去除冗余，修正 _FORTIFY_SOURCE 拼写）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")

# 安全链接参数（链接器专属选项移至 CMAKE_EXE_LINKER_FLAGS）
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie -Wl,-z,lazy")

if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  # 安全测试选项（ASAN 需配合 -O2 使 _FORTIFY_SOURCE 生效）
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
endif()

# 代码覆盖率编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage")

# 本项目固定使用 Qt6 + DTK6
message(STATUS "Testing with Qt6")
find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets DBus Concurrent Svg PrintSupport Test Quick Qml)
find_package(Dtk6 REQUIRED COMPONENTS Core Widget Gui Declarative)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)

# OCR 相关依赖
pkg_search_module(OCR_PLUGIN REQUIRED deepin-ocr-plugin-manager)
pkg_check_modules(InferenceEngine REQUIRED ncnn opencv_mobile)
include_directories(${OCR_PLUGIN_INCLUDE_DIRS})

include_directories(${GTEST_INCLUDE_DIRS})

# 保证 src 目录及其子目录的头文件全局可见，与 src/CMakeLists.txt 保持一致
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/src)
include_directories(${CMAKE_SOURCE_DIR}/src/src/unionimage)
include_directories(${CMAKE_SOURCE_DIR}/src/src/imagedata)
include_directories(${CMAKE_SOURCE_DIR}/src/src/dbus)
include_directories(${CMAKE_SOURCE_DIR}/src/src/declarative)
include_directories(${CMAKE_SOURCE_DIR}/src/src/ocr)
include_directories(${CMAKE_SOURCE_DIR}/src/src/printdialog)
include_directories(${CMAKE_SOURCE_DIR}/src/src/utils)
include_directories(${CMAKE_SOURCE_DIR}/src/src/misc)
include_directories(src)

set(PROJECT_NAME_TEST
    ${PROJECT_NAME}-test)

# 收集源码文件，仅扫描 src/src/ 子目录（程序入口 src/main.cpp 不在其中）
file(GLOB_RECURSE IMAGEVIEWER_SRC_TEST ${CMAKE_SOURCE_DIR}/src/src/*.cpp)
# 防御性排除 main.cpp，避免 glob 规则变更后产生 main 函数重定义冲突
list(REMOVE_ITEM IMAGEVIEWER_SRC_TEST
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/src/main.cpp"
)

# 收集测试源文件
file(GLOB_RECURSE IMAGEVIEWER_UT_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

# 生成测试可执行程序
add_executable(${PROJECT_NAME_TEST}
    ${IMAGEVIEWER_SRC_TEST}
    ${IMAGEVIEWER_UT_SRC}
    )

# 生成测试可执行程序的依赖库
target_link_libraries(${PROJECT_NAME_TEST} gtest gtest_main -pthread)

target_link_libraries(${PROJECT_NAME_TEST}
    Dtk6::Core
    Dtk6::Gui
    Dtk6::Widget
    Dtk6::Declarative
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Concurrent
    Qt6::Svg
    Qt6::PrintSupport
    Qt6::Test
    Qt6::Quick
    Qt6::Qml
    ${InferenceEngine_LIBRARIES}
    ${OCR_PLUGIN_LIBRARIES}
    GL
    pthread
)

##------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND ${CMAKE_BINARY_DIR}/tests/${PROJECT_NAME_TEST}
    COMMAND echo " =================== TEST END ==================== "
)

##'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})
