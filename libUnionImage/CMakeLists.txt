#定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

project(libUnionImage)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#Qt需要的包
set(QtModule Core Gui Widgets Svg Concurrent)

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

#设置输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../viewer/lib)

#包含目录
include_directories(UnionImage)
include_directories(UnionImage/giflib)
include_directories(UnionImage/Source)
include_directories(UnionImage/wrapper)
include_directories(UnionImage/Source/FreeImage)
include_directories(UnionImage/Source/FreeImageToolkit)
include_directories(UnionImage/Source/LibJXR/image/decode)
include_directories(UnionImage/Source/LibJXR/image/sys)
include_directories(UnionImage/Source/LibJXR/image/x86)
include_directories(UnionImage/Source/LibJXR/common/include)
include_directories(UnionImage/Source/LibJXR/jxrgluelib)
include_directories(UnionImage/Source/LibOpenJPEG)
include_directories(UnionImage/Source/LibPNG)
include_directories(UnionImage/Source/LibRawLite)
include_directories(UnionImage/Source/LibRawLite/internal)
include_directories(UnionImage/Source/LibRawLite/libraw)
include_directories(UnionImage/Source/LibTIFF4)
include_directories(UnionImage/Source/LibWebP)
include_directories(UnionImage/Source/LibWebP/src)
include_directories(UnionImage/Source/LibWebP/src/dec)
include_directories(UnionImage/Source/LibWebP/src/demux)
include_directories(UnionImage/Source/LibWebP/src/dsp)
include_directories(UnionImage/Source/LibWebP/src/enc)
include_directories(UnionImage/Source/LibWebP/src/mux)
include_directories(UnionImage/Source/LibWebP/src/utils)
include_directories(UnionImage/Source/LibWebP/src/webp)
include_directories(UnionImage/Source/Metadata)
include_directories(UnionImage/Source/OpenEXR)
include_directories(UnionImage/Source/OpenEXR/Half)
include_directories(UnionImage/Source/OpenEXR/Iex)
include_directories(UnionImage/Source/OpenEXR/IexMath)
include_directories(UnionImage/Source/OpenEXR/IlmImf)
include_directories(UnionImage/Source/OpenEXR/IlmThread)
include_directories(UnionImage/Source/OpenEXR/Imath)
include_directories(UnionImage/Source/OpenEXR/ilmbase)
include_directories(UnionImage/Source/OpenEXR/openexr)
include_directories(UnionImage/Source/ZLib)
include_directories(UnionImage/wrapper)

#库目录
aux_source_directory(UnionImage allSources)
aux_source_directory(UnionImage/giflib allSources)
aux_source_directory(UnionImage/Source allSources)
aux_source_directory(UnionImage/wrapper allSources)
aux_source_directory(UnionImage/Source/FreeImage allSources)
aux_source_directory(UnionImage/Source/FreeImageToolkit allSources)
aux_source_directory(UnionImage/Source/LibJXR/image/decode allSources)
aux_source_directory(UnionImage/Source/LibJXR/image/sys allSources)
aux_source_directory(UnionImage/Source/LibJXR/image/x86 allSources)
aux_source_directory(UnionImage/Source/LibJXR/common/include allSources)
aux_source_directory(UnionImage/Source/LibJXR/jxrgluelib allSources)
aux_source_directory(UnionImage/Source/LibOpenJPEG allSources)
aux_source_directory(UnionImage/Source/LibPNG allSources)
aux_source_directory(UnionImage/Source/LibRawLite allSources)
aux_source_directory(UnionImage/Source/LibRawLite/internal allSources)
aux_source_directory(UnionImage/Source/LibRawLite/libraw allSources)
aux_source_directory(UnionImage/Source/LibRawLite/src allSources)
aux_source_directory(UnionImage/Source/LibTIFF4 allSources)
aux_source_directory(UnionImage/Source/LibWebP allSources)
aux_source_directory(UnionImage/Source/LibWebP/src allSources)
aux_source_directory(UnionImage/Source/LibWebP/src/dec allSources)
aux_source_directory(UnionImage/Source/LibWebP/src/demux allSources)
aux_source_directory(UnionImage/Source/LibWebP/src/dsp allSources)
aux_source_directory(UnionImage/Source/LibWebP/src/enc allSources)
aux_source_directory(UnionImage/Source/LibWebP/src/mux allSources)
aux_source_directory(UnionImage/Source/LibWebP/src/utils allSources)
aux_source_directory(UnionImage/Source/LibWebP/src/webp allSources)
aux_source_directory(UnionImage/Source/Metadata allSources)
aux_source_directory(UnionImage/Source/OpenEXR allSources)
aux_source_directory(UnionImage/Source/OpenEXR/Half allSources)
aux_source_directory(UnionImage/Source/OpenEXR/Iex allSources)
aux_source_directory(UnionImage/Source/OpenEXR/IexMath allSources)
aux_source_directory(UnionImage/Source/OpenEXR/IlmImf allSources)
aux_source_directory(UnionImage/Source/OpenEXR/IlmThread allSources)
aux_source_directory(UnionImage/Source/OpenEXR/Imath allSources)
aux_source_directory(UnionImage/Source/OpenEXR/ilmbase allSources)
aux_source_directory(UnionImage/Source/OpenEXR/openexr allSources)
aux_source_directory(UnionImage/Source/ZLib allSources)
aux_source_directory(UnionImage/wrapper allSources)

#需要打开的头文件
FILE (GLOB allHeaders "./UnionImage/*.h")

#需要打开的代码文件
FILE (GLOB allSources
    "./UnionImage/*.cpp"                                   "./UnionImage/*.c"
    "./UnionImage/giflib/*.cpp"                            "./UnionImage/giflib/*.c"
    "./UnionImage/Source/*.cpp"                            "./UnionImage/Source/*.c"
    "./UnionImage/Source/FreeImage/*.cpp"                  "./UnionImage/Source/FreeImage/*.c"
    "./UnionImage/Source/FreeImageToolkit/*.cpp"           "./UnionImage/Source/FreeImageToolkit/*.c"
    "./UnionImage/Source/LibJXR/image/decode/*.cpp"        "./UnionImage/Source/LibJXR/image/decode/*.c"
    "./UnionImage/Source/LibJXR/image/encode/*.cpp"        "./UnionImage/Source/LibJXR/image/encode/*.c"
    "./UnionImage/Source/LibJXR/image/sys/*.cpp"           "./UnionImage/Source/LibJXR/image/sys/*.c"
    "./UnionImage/Source/LibJXR/image/x86/*.cpp"           "./UnionImage/Source/LibJXR/image/x86/*.c"
    "./UnionImage/Source/LibJXR/jxrgluelib/*.cpp"          "./UnionImage/Source/LibJXR/jxrgluelib/*.c"
    "./UnionImage/Source/LibOpenJPEG/*.cpp"                "./UnionImage/Source/LibOpenJPEG/*.c"
    "./UnionImage/Source/LibPNG/*.cpp"                     "./UnionImage/Source/LibPNG/*.c"

    "./UnionImage/Source/LibRawLite/internal/dcraw_common.cpp"
    "./UnionImage/Source/LibRawLite/internal/dcraw_fileio.cpp"
    "./UnionImage/Source/LibRawLite/internal/demosaic_packs.cpp"
    "./UnionImage/Source/LibRawLite/src/libraw_c_api.cpp"
    "./UnionImage/Source/LibRawLite/src/libraw_cxx.cpp"
    "./UnionImage/Source/LibRawLite/src/libraw_datastream.cpp"

    "./UnionImage/Source/LibTIFF4/*.cpp"                   "./UnionImage/Source/LibTIFF4/*.c"
    "./UnionImage/Source/LibWebP/src/dec/*.cpp"            "./UnionImage/Source/LibWebP/src/dec/*.c"
    "./UnionImage/Source/LibWebP/src/demux/*.cpp"          "./UnionImage/Source/LibWebP/src/demux/*.c"
    "./UnionImage/Source/LibWebP/src/dsp/*.cpp"            "./UnionImage/Source/LibWebP/src/dsp/*.c"
    "./UnionImage/Source/LibWebP/src/enc/*.cpp"            "./UnionImage/Source/LibWebP/src/enc/*.c"
    "./UnionImage/Source/LibWebP/src/mux/*.cpp"            "./UnionImage/Source/LibWebP/src/mux/*.c"
    "./UnionImage/Source/LibWebP/src/utils/*.cpp"          "./UnionImage/Source/LibWebP/src/utils/*.c"
    "./UnionImage/Source/LibWebP/src/webp/*.cpp"           "./UnionImage/Source/LibWebP/src/webp/*.c"
    "./UnionImage/Source/Metadata/*.cpp"                   "./UnionImage/Source/Metadata/*.c"
    "./UnionImage/Source/OpenEXR/*.cpp"                    "./UnionImage/Source/OpenEXR/*.c"
    "./UnionImage/Source/OpenEXR/Half/*.cpp"               "./UnionImage/Source/OpenEXR/Half/*.c"
    "./UnionImage/Source/OpenEXR/Iex/*.cpp"                "./UnionImage/Source/OpenEXR/Iex/*.c"
    "./UnionImage/Source/OpenEXR/IexMath/*.cpp"            "./UnionImage/Source/OpenEXR/IexMath/*.c"
    "./UnionImage/Source/OpenEXR/IlmImf/*.cpp"             "./UnionImage/Source/OpenEXR/IlmImf/*.c"
    "./UnionImage/Source/OpenEXR/IlmThread/*.cpp"          "./UnionImage/Source/OpenEXR/IlmThread/*.c"
    "./UnionImage/Source/OpenEXR/Imath/*.cpp"              "./UnionImage/Source/OpenEXR/Imath/*.c"
    "./UnionImage/Source/OpenEXR/ilmbase/*.cpp"            "./UnionImage/Source/OpenEXR/ilmbase/*.c"
    "./UnionImage/Source/OpenEXR/openexr/*.cpp"            "./UnionImage/Source/OpenEXR/openexr/*.c"
    "./UnionImage/Source/ZLib/*.cpp"                       "./UnionImage/Source/ZLib/*.c"
    "./UnionImage/wrapper/*.cpp"                           "./UnionImage/wrapper/*.c"
    )

#编译为库
add_library(UnionImage SHARED ${allHeaders} ${allSources})

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#使用库
qt5_use_modules(UnionImage ${QtModule})

#将库安装到指定位置
set(PREFIX /usr)
set(UnionImageDir ${PREFIX}/lib/)

install(TARGETS UnionImage LIBRARY DESTINATION ${UnionImageDir})

execute_process(
COMMAND ln -s ${UnionImageDir}/libUnionImage.so libUnionImage.so.1
COMMAND ln -s ${UnionImageDir}/libUnionImage.so libUnionImage.so.1.0
COMMAND ln -s ${UnionImageDir}/libUnionImage.so libUnionImage.so.1.0.0
)

target_link_libraries(UnionImage)
