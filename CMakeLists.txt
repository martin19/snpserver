cmake_minimum_required(VERSION 3.16)
project(snpserver)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

#include(FindProtobuf)
#find_package(Protobuf)
#include_directories(${PROTOBUF_INCLUDE_DIR})

set(PROTOBUF_LIBRARY
    "C:\\lib\\protobuf-3.21.12\\cmake-build-release\\libprotobuf.a"
)

find_package(LibWebSockets)
if(LIBWEBSOCKETS_FOUND)
    add_definitions(-DHAVE_LIBWEBSOCKETS)
endif(LIBWEBSOCKETS_FOUND)

find_package(OPENH264 REQUIRED)
if(OPENH264_FOUND)
    add_definitions(-DHAVE_LIBOPENH264)
    include_directories(
            /usr/include
            ${OPENH264_INCLUDE_DIR}
            /usr/include/libdrm #todo move this
    )
    set(OPENH264_SOURCES
            src/stream/video/SnpEncoderOpenH264.cpp
            )
endif(OPENH264_FOUND)

find_package(VA)
if(VA_FOUND)
    add_definitions(-DHAVE_LIBVA)
    include_directories(
            ${VA_INCLUDE_DIR}
            /usr/include/libdrm #todo move this
    )
    set(VA_SOURCES
            src/stream/video/SnpEncoderVaH264.cpp
            src/stream/video/h264/VaBitstream.cpp
    )
endif(VA_FOUND)

find_package(MMALH264)
if(MMALH264_FOUND)
    add_definitions(-DHAVE_LIBMMALH264)
    include_directories(
            /usr/include
            ${MMALH264_INCLUDE_DIR}
            /usr/include/libdrm #todo move this
    )
    set(MMALH264_SOURCES
        src/stream/video/SnpEncoderMmalH264.cpp
    )
endif(MMALH264_FOUND)

include_directories(
    src
    extern/openh264/include
    extern/protobuf-3.21.12
    extern/abseil-cpp
    LIBWEBSOCKETS_INCLUDE_DIR
)

if(UNIX)
    add_definitions(-DHAVE_LIBDRM)
    add_definitions(-DHAVE_LIBX11)
    add_definitions(-DHAVE_LIBOPENGL)
    set(LIBDRM_FOUND TRUE)
    set(LIBX11_FOUND TRUE)
    set(LIBGL_FOUND TRUE)
    add_library(libdrm SHARED IMPORTED)
    #raspberry
    #set_target_properties(libdrm PROPERTIES IMPORTED_LOCATION /usr/local/lib/arm-linux-gnueabihf/libdrm.so.2)
    #set_target_properties(libdrm PROPERTIES VERSION 2 SOVERSION 2.4)
    #devbox
    set_target_properties(libdrm PROPERTIES IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libdrm.so)
endif(UNIX)

if(LIBX11_FOUND)
    set(X11_SOURCES
        src/stream/input/SnpSinkX11Mouse.cpp
        src/stream/input/SnpSinkX11Keyboard.cpp
        src/stream/video/SnpSourceX11.cpp
        src/stream/input/SnpSourceX11Cursor.cpp
    )
endif(LIBX11_FOUND)

if(LIBDRM_FOUND)
    set(DRM_SOURCES
        src/util/DrmUtil.cpp
    )
endif(LIBDRM_FOUND)

if(LIBGL_FOUND)
    set(GL_SOURCES
        src/util/DrmUtil.cpp
        src/stream/video/SnpSourceGL.cpp
            src/stream/network/SnpSinkNetworkTcp.cpp src/stream/network/SnpSinkNetworkTcp.h src/stream/network/sockets.h src/stream/video/SnpSourceDummy.cpp src/stream/video/SnpSourceDummy.h)
endif(LIBGL_FOUND)

if(LIBWEBSOCKETS_FOUND)
    set(WEBSOCKETS_SOURCES
        src/network/SnpClientWebsocket.cpp
        src/network/SnpWebsocket.cpp
        src/stream/network/SnpSinkNetworkWebsocket.cpp
    )
endif(LIBWEBSOCKETS_FOUND)

add_executable(snpserver
    main.cpp
    src/network/snappyv1.pb.cc
    src/util/loguru.cpp
    src/util/TimeUtil.cpp
    src/util/VideoUtil.cpp
    src/util/PropertyUtil.cpp
    # stream
    src/stream/SnpProperty.cpp
    src/stream/SnpComponent.cpp
    src/stream/SnpPort.cpp
    src/stream/SnpPipe.cpp
    src/stream/SnpPipeFactory.cpp
    # video stream
    src/stream/video/SnpSourceDummy.cpp
    # input stream
    src/stream/input/keymap_atset1_linux.c
    # network sink/source
    src/stream/network/SnpSourceNetwork.cpp
    src/stream/network/SnpSinkNetworkTcp.cpp
    # file sink/source
    src/stream/file/SnpSinkFile.cpp
    ${X11_SOURCES}
    ${DRM_SOURCES}
    ${GL_SOURCES}
    ${MMALH264_SOURCES}
    ${OPENH264_SOURCES}
    ${VA_SOURCES}
)

if(VA_FOUND)
    target_link_libraries(snpserver ${VA_LIBRARIES})
endif(VA_FOUND)

if(LIBWEBSOCKETS_FOUND)
    target_link_libraries(snpserver ${LIBWEBSOCKETS_LIBRARIES})
endif(LIBWEBSOCKETS_FOUND)

if(MMALH264_FOUND)
    target_link_libraries(snpserver ${MMALH264_LIBRARIES})
endif(MMALH264_FOUND)

if(LIBDRM_FOUND)
    target_link_libraries(snpserver libdrm)
endif(LIBDRM_FOUND)

if(X11_FOUND)
    target_link_libraries(snpserver X11 Xfixes)
endif(X11_FOUND)

if(OPENGL_FOUND)
    target_link_libraries(snpserver EGL GLESv2)
endif(OPENGL_FOUND)

if(WIN32)
   target_link_libraries(snpserver wsock32 ws2_32)
endif(WIN32)

target_link_libraries(snpserver
    ${PROTOBUF_LIBRARY}
    ${OPENH264_LIBRARIES}
    pthread
    #dl
)