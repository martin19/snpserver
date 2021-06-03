cmake_minimum_required(VERSION 3.16)
project(snpserver)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

find_package(LibWebSockets REQUIRED)

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
    LIBWEBSOCKETS_INCLUDE_DIR
)

#TODO!
add_library(libdrm SHARED IMPORTED)
set_target_properties(libdrm PROPERTIES IMPORTED_LOCATION /usr/local/lib/arm-linux-gnueabihf/libdrm.so.2)
set_target_properties(libdrm PROPERTIES VERSION 2 SOVERSION 2.4)

add_executable(snpserver
    main.cpp
    src/network/snappyv1.pb.cc
    src/network/SnpClient.cpp
    src/network/SnpSocket.cpp
    src/util/loguru.cpp
    # stream
    src/stream/SnpComponent.cpp
    src/stream/SnpPort.cpp
    # video stream
    src/stream/video/SnpSourceModesetting.cpp
    src/stream/video/SnpSourceGL.cpp
    # input stream
    src/stream/input/SnpSinkMouse.cpp
    src/stream/input/keymap_atset1_linux.c
    src/stream/input/SnpSinkKeyboard.cpp
    src/stream/input/SnpSourceCursor.cpp
    # network sink/source
    src/stream/network/SnpSourceNetwork.cpp
    src/stream/network/SnpSinkNetwork.cpp
    ${MMALH264_SOURCES}

    #in flux
        src/stream/SnpPipe.cpp
)

target_link_libraries(snpserver
    ${LIBWEBSOCKETS_LIBRARIES}
    ${PROTOBUF_LIBRARY}
    ${MMALH264_LIBRARIES}
    pthread
    libdrm
    dl
    X11
    Xfixes
    EGL
    GLESv2
)

#target_link_libraries(snappyVnc PRIVATE spdlog::spdlog_header_only)