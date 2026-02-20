cmake_minimum_required(VERSION 3.13)

# 玲珑构建依赖目录
include_directories(${CMAKE_INSTALL_PREFIX}/include)
include_directories(${CMAKE_INSTALL_PREFIX}/include/deepin-ocr-plugin-manager)

set(APP_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin/)
set(BIN_NAME ${CMAKE_PROJECT_NAME})
list(APPEND RESOURCES)

find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS
    Quick
    DBus
    Concurrent
    Svg
    PrintSupport
    LinguistTools
)

find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS
    Widget
    Declarative
)

# OCR
find_package(PkgConfig REQUIRED)
pkg_search_module(OCR_PLUGIN REQUIRED deepin-ocr-plugin-manager)
pkg_check_modules(HEIF REQUIRED libheif)
pkg_check_modules(InferenceEngine REQUIRED ncnn opencv_mobile)
include_directories(${OCR_PLUGIN_INCLUDE_DIRS})

# 保证 src 目录下头文件全局可见
include_directories(src)

# 源文件
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS "./*.h" "./*.cpp")
file(GLOB_RECURSE QMLSRC ./*.qml)

qt_add_resources(SRCS res.qrc)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wl,--as-need -fPIE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=incompatible-pointer-types -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wl,--as-need -fPIE -Wno-error=incompatible-pointer-types")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

if(NOT BUILD_WITH_QT6)
    qt_add_resources(APPEND SRCS deepin-image-viewer.qrc)

    find_package(Qt5QuickCompiler CONFIG REQUIRED)
    qtquick_compiler_add_resources(RCC_SOURCES ${RESOURCES})
    list(APPEND SRCS ${RCC_SOURCES})
endif()

if(NOT(${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    # 调整优化级别
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

    # Translation
    file(GLOB TS LIST_DIRECTORIES false translations/${CMAKE_PROJECT_NAME}*.ts)
    set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)
    qt_create_translation(QM ${SRCS} ${QMLSRC} ${TS})
    list(APPEND SRCS ${QM})
endif()

if(NOT BUILD_WITH_QT6)
    add_executable(${BIN_NAME}
        ${SRCS}
        ${RCC_SOURCES}
    )

else()
    qt_policy(SET QTP0001 NEW)

    # Executable definition
    add_executable(${BIN_NAME}
        main.cpp # Add main.cpp explicitly
        ${SRCS} # Add other C++ sources and generated QM files
    )

    # QML Module definition
    qt_add_qml_module(${BIN_NAME}
        URI IVModule # Define a URI for the QML module
        VERSION 1.0
        QML_FILES
            qml/main.qml
            qml/FullImageView.qml
            qml/ImageAnimation.qml
            qml/ImageViewer.qml
            qml/MainStack.qml
            qml/NavigationWidget.qml
            qml/OpenImageWidget.qml
            qml/ReName.qml
            qml/SFadeInOut.qml
            qml/SliderShow.qml
            qml/ThumbnailListView.qml
            qml/ViewRightMenu.qml
            qml/ViewTopTitle.qml
            qml/Dialog/RemoveDialog.qml
            qml/ImageDelegate/BaseImageDelegate.qml
            qml/ImageDelegate/DamagedImageDelegate.qml
            qml/ImageDelegate/DynamicImageDelegate.qml
            qml/ImageDelegate/MultiImageDelegate.qml
            qml/ImageDelegate/NonexistImageDelegate.qml
            qml/ImageDelegate/NormalImageDelegate.qml
            qml/ImageDelegate/SvgImageDelegate.qml
            qml/ImageDelegate/ViewDelegateLoader.qml
            qml/InformationDialog/ElideLabel.qml
            qml/InformationDialog/InformationDialog.qml
            qml/InformationDialog/PropertyActionItemDelegate.qml
            qml/InformationDialog/PropertyItem.qml
            qml/InformationDialog/PropertyItemDelegate.qml
            qml/LiveText/LiveBlock.qml
            qml/LiveText/LiveBlockController.js
            qml/LiveText/LiveBlockRubberBand.qml
            qml/LiveText/LiveTextWidget.qml
            qml/ThumbnailDelegate/BaseThumbnailDelegate.qml
            qml/ThumbnailDelegate/MultiThumnailDelegate.qml
            qml/ThumbnailDelegate/NormalThumbnailDelegate.qml
            qml/ThumbnailDelegate/ThumbnailImage.qml
            qml/Utils/FloatingNotice.qml
            qml/Utils/ImageInputHandler.qml
            qml/Utils/RightMenuItem.qml

        NO_PLUGIN
        NO_GENERATE_PLUGIN_SOURCE # We are building an executable, not a plugin library
        NO_PLUGIN_OPTIONAL # Ensure the module is built

        # ENABLE_TYPE_COMPILER
        NO_GENERATE_EXTRA_QMLDIRS
    )

    # commercial-only
    # set_target_properties(${BIN_NAME} PROPERTIES
    #     QT_QMLCACHEGEN_DIRECT_CALLS ON
    # )
endif()

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Svg

    # 移除
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Declarative
    GL
    pthread
    ${InferenceEngine_LIBRARIES}
    ${OCR_PLUGIN_LIBRARIES}
    ${HEIF_LIBRARIES}
)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    TARGET_COMPILE_DEFINITIONS(${BIN_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
endif()

include_directories(${HEIF_INCLUDE_DIRS})
link_directories(${HEIF_LIBRARY_DIRS})


set(APPSHAREDIR ${CMAKE_INSTALL_PREFIX}/share/deepin-image-viewer)

# Add install rule
install(TARGETS ${BIN_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
install(FILES ${CMAKE_PROJECT_NAME}.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES ./icon/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
install(FILES com.deepin.imageViewer.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services/)

# translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${APPSHAREDIR}
    FILES_MATCHING PATTERN "*.qm")

# manual
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/assets/deepin-image-viewer
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-manual/manual-assets/application/
    FILES_MATCHING PATTERN "*")
#context-menu conf 文管右键菜单配置文件
install(FILES ${PROJECT_SOURCE_DIR}/src/misc/context-menus/deepin-print-pictures.conf
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications/context-menus/)
