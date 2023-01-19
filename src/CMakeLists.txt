cmake_minimum_required(VERSION 3.1)

#玲珑构建依赖目录
include_directories(${CMAKE_INSTALL_PREFIX}/include)
include_directories(${CMAKE_INSTALL_PREFIX}/include/deepin-ocr-plugin-manager)

set(APP_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin/)
set(BIN_NAME ${CMAKE_PROJECT_NAME})
list (APPEND RESOURCES)

# Set sources
set(SRC
    main.cpp
    )

# 源文件
file(GLOB_RECURSE SRCS
    ./src/*.h
    ./src/*.cpp
    )

file(GLOB_RECURSE QMLSRC
    ./*.qml
    )

# Find the Qt5Quick library
find_package(Qt5Quick CONFIG REQUIRED)
find_package(Qt5QuickCompiler REQUIRED)
find_package(Qt5DBus CONFIG REQUIRED)
find_package(Qt5Concurrent CONFIG REQUIRED)
find_package(Qt5Svg CONFIG REQUIRED)
find_package(Qt5PrintSupport CONFIG REQUIRED)
find_package(Qt5LinguistTools CONFIG REQUIRED)

qtquick_compiler_add_resources(RCC_SOURCES ${RESOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget
    )

# OCR
pkg_search_module(OCR_PLUGIN REQUIRED deepin-ocr-plugin-manager)
include_directories(${OCR_PLUGIN_INCLUDE_DIRS})

# 保证 src 目录下头文件全局可见
include_directories(src)

# Translation
file(GLOB TS LIST_DIRECTORIES false translations/${CMAKE_PROJECT_NAME}*.ts)
set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)

qt5_create_translation(QM ${SRC} ${SRCS} ${RCC_SOURCES} ${QMLSRC} ${GLOB})

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(QM ${SRC} ${SRCS} ${RCC_SOURCES} ${QMLSRC} ${TS})
endif ()

# 仅生成 EXE 文件，用以兼容新(5.6.0+dev-1及以上)旧两版本DtkDeclarative库
# Add EXE
add_executable(${BIN_NAME}
    ${SRC}
    ${SRCS}
    ${RCC_SOURCES}
    ${QM}
    deepin-image-viewer.qrc
    res.qrc
    )

target_include_directories(${BIN_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})
target_link_libraries(${BIN_NAME}
    ${DtkDeclarative_LIBRARIES}
    Qt5::Quick Qt5::PrintSupport Qt5::Gui Qt5::Qml Qt5::Core Qt5::DBus Qt5::Concurrent Qt5::Svg
    GL pthread freeimage
    ${3rd_lib_LIBRARIES}
    ${OCR_PLUGIN_LIBRARIES}
    )

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    TARGET_COMPILE_DEFINITIONS(${BIN_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
endif ()

set_target_properties(${BIN_NAME} PROPERTIES INSTALL_RPATH ${DTK_QML_APP_PLUGIN_PATH})

set(APPSHAREDIR  ${CMAKE_INSTALL_PREFIX}/share/deepin-image-viewer)

# Add install rule
install(TARGETS ${BIN_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
install(FILES ${CMAKE_PROJECT_NAME}.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES ./icon/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
install(FILES com.deepin.imageViewer.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-d/services/)

#translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${APPSHAREDIR}
    FILES_MATCHING PATTERN "*.qm")

#manual
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/assets/deepin-image-viewer
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-manual/manual-assets/application/
    FILES_MATCHING PATTERN "*")
