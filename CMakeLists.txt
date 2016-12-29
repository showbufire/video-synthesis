cmake_minimum_required(VERSION 3.6.3)
project(video_synthesis)

set(LIB_SOURCES src/VideoEncoder.cpp src/VideoDecoder.cpp)

set(FFMPEG_LIBS "libavdevice libavformat libavfilter libavcodec libswresample libswscale libavutil")

set(PKG_CONFIG_COMMAND1 pkg-config --cflags ${FFMPEG_LIBS})
execute_process(COMMAND ${PKG_CONFIG_COMMAND1} OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE FFMPEG_LIBS_CFLAGS)

set(PKG_CONFIG_COMMAND2 pkg-config --libs ${FFMPEG_LIBS})
execute_process(COMMAND ${PKG_CONFIG_COMMAND2} OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE FFMPEG_LIBS_LDLIBS)

set(GCC_CXXFLAGS "-Wall -g -std=c++11 ${FFMPEG_LIBS_CFLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_CXXFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FFMPEG_LIBS_LDLIBS}")

# img2vid
set(IMG2VID_SOURCES src/img2vid.cpp ${LIB_SOURCES})
add_executable(img2vid ${IMG2VID_SOURCES})

# imgimg
set(IMGIMG_SOURCES src/imgimg.cpp ${LIB_SOURCES})
add_executable(imgimg ${IMGIMG_SOURCES})

# imgxvid
set(IMGXVID_SOURCES src/imgxvid.cpp ${LIB_SOURCES})
add_executable(imgxvid ${IMGXVID_SOURCES})

# imgonvid
set(IMGONVID_SOURCES src/imgonvid.cpp ${LIB_SOURCES})
add_executable(imgonvid ${IMGONVID_SOURCES})
