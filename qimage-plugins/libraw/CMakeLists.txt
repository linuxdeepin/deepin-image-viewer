project(xraw VERSION 0.1.0)
set(CMAKE_AUTOMOC ON)
set(CMD_NAME xraw)
#set(CMAKE_CXX_STANDARD 11)



ADD_DEFINITIONS(-DTORRENT_LIBRARY)
ADD_DEFINITIONS(-DNO_FREETYPE)
ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
ADD_DEFINITIONS(-DHCPP_LEGACY_API)
add_definitions(-D_LIBXRAW_)
find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)

pkg_check_modules(RAW REQUIRED libraw)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

 # 加速编译优化参数
 if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack")
     SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z noexecstack")
     SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
endif ()

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include(GNUInstallDirs)
include_directories(${PROJECT_BINARY_DIR})

list(APPEND SRCS
    main.cpp
    rawiohandler.cpp
    datastream.cpp)

add_library(${CMD_NAME} SHARED ${SRCS})

set_target_properties(${CMD_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)
#target_compile_definitions(${CMD_NAME} PRIVATE BUILDING_${CMD_NAME})

target_include_directories(xraw PUBLIC ${RAW_INCLUDE_DIRS}  ${Qt5Gui_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} )

target_link_libraries(${CMD_NAME} Qt5::Core Qt5::Gui raw)

install(TARGETS ${CMD_NAME} DESTINATION ${Qt5Core_DIR}/../../qt5/plugins/imageformats)


QT5_USE_MODULES(${PROJECT_NAME} Core Gui)
