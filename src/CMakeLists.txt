set(BIN_NAME ${CMAKE_PROJECT_NAME})
# Set sources
set(SRC
  main.cpp
)

file(GLOB OCR
  ./src/ocr/*.h
  ./src/ocr/*.cpp
)

file(GLOB UIONIMAGE
  ./src/unionimage/*.h
  ./src/unionimage/*.cpp
)

file(GLOB DBUS
  ./src/dbus/*.h
  ./src/dbus/*.cpp
)

file(GLOB PRINTDIALOG
  ./src/printdialog/*.h
  ./src/printdialog/*.cpp
)

file(GLOB UTILS
  ./src/utils/*.h
  ./src/utils/*.cpp
)

file(GLOB TOP
  ./src/*.h
  ./src/*.cpp
)

set(SRCS
  ${PRINTDIALOG}
  ${DBUS}
  ${UIONIMAGE}
  ${UTILS}
  ${TOP}
)

if(ENABLEOCR)
  list(APPEND SRCS
    ${OCR}
  )
endif()

file(GLOB_RECURSE QMLSRC
  ./*.qml
)

pkg_search_module(LIBRAW REQUIRED IMPORTED_TARGET libraw)
qtquick_compiler_add_resources(RCC_SOURCES ${RESOURCES})

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

target_link_libraries(${BIN_NAME}
    ${DtkDeclarative_LIBRARIES}
    Qt5::Quick
    Qt5::PrintSupport
    Qt5::Gui
    Qt5::Qml
    Qt5::Core
    Qt5::DBus
    Qt5::Concurrent
    Qt5::Svg
    GL pthread freeimage
    ${DtkWidget_LIBRARIES}
    PkgConfig::LIBRAW
  )
if(ENABLEOCR)
  target_link_libraries(${BIN_NAME}
    PkgConfig::OCR_PLUGIN
  )

endif()
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    TARGET_COMPILE_DEFINITIONS(${BIN_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
endif ()

set_target_properties(${BIN_NAME} PROPERTIES INSTALL_RPATH ${DTK_QML_APP_PLUGIN_PATH})

set(APPSHAREDIR  ${CMAKE_INSTALL_PREFIX}/share/deepin-image-viewer)

# Add install rule
install(TARGETS ${BIN_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
install(FILES ${CMAKE_PROJECT_NAME}.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES ./icon/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
install(FILES com.deepin.imageViewer.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services/)

#translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${APPSHAREDIR}
    FILES_MATCHING PATTERN "*.qm")

#manual
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/assets/deepin-image-viewer
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-manual/manual-assets/application/
    FILES_MATCHING PATTERN "*")
