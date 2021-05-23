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
        src/stream/SnpEncoderMmalH264.cpp
            src/websocket/MiniWebsocket.cpp src/websocket/MiniWebsocket.h)
endif(MMALH264_FOUND)

include_directories(
        src
        LIBWEBSOCKETS_INCLUDE_DIR
)

add_executable(snpserver
    main.cpp
    src/network/snappyv1.pb.cc
    src/network/SnpProtocol.cpp
    src/network/SnpClient.cpp
    src/network/SnpSocket.cpp
    src/util/loguru.cpp
    src/network/SnpProtocol.cpp
    src/network/SnpProtocol.h
    src/stream/SnpSource.cpp
    src/stream/SnpEncoder.cpp
    src/stream/SnpSink.cpp
    src/stream/SnpChain.cpp
    src/stream/SnpSourceModesetting.cpp
    src/stream/SnpSinkNetwork.cpp
    ${MMALH264_SOURCES}
)

target_link_libraries(snpserver
    ${LIBWEBSOCKETS_LIBRARIES}
    ${PROTOBUF_LIBRARY}
    ${MMALH264_LIBRARIES}
    pthread
    drm
    dl
)

#target_link_libraries(snappyVnc PRIVATE spdlog::spdlog_header_only)