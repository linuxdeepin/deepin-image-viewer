cmake_minimum_required(VERSION 3.0.0)

project(deepin-image-viewer)

execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_package(PkgConfig REQUIRED)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

message(STATUS ${CMAKE_CXX_FLAGS})


option(DOTEST "option for test" OFF)

if(DOTEST)
    add_subdirectory(tests)
endif()


#add_subdirectory(libimage-viewer)
add_subdirectory(src)
add_subdirectory(qimage-plugins)
