cmake_minimum_required(VERSION 3.16)
project(snpserver)

set(PROTO_TARGET_NAME protobuf_sources)
set(SERVER_TARGET_NAME snpserver)
set(CLIENT_TARGET_NAME snpclient)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

# library: protobuf 3.21.12
set(protobuf_BUILD_TESTS OFF)
add_subdirectory("extern/protobuf-3.21.12")

# library: QT 6.7.2
set(Qt6_DIR C:/Qt/6.7.2/mingw_64/lib/cmake/Qt6/)
set(CMAKE_PREFIX_PATH C:/Qt/6.7.2/mingw_64/lib/cmake)
find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    REQUIRED
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
        src/stream/video/SnpDecoderOpenH264.cpp
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
    extern/protobuf-3.21.12/src
    LIBWEBSOCKETS_INCLUDE_DIR
)

if(UNIX)
    add_definitions(-DHAVE_LIBDRM)
    add_definitions(-DHAVE_LIBX11)
    add_definitions(-DHAVE_LIBOPENGL)
    set(LIBDRM_FOUND TRUE)
    set(LIBX11_FOUND TRUE)
    set(LIBGL_FOUND TRUE)
    add_library(libdrm SHARED IMPORTED src/stream/video/h264/OpenH264Api.h src/stream/output/SnpSinkDisplay.cpp src/stream/output/SnpSinkDisplay.h)
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
            src/stream/network/SnpSinkNetworkTcp.cpp src/stream/network/SnpSinkNetworkTcp.h src/stream/network/sockets.h src/stream/video/SnpSourceDummy.cpp src/stream/video/SnpSourceDummy.h src/snpclient.cpp src/stream/video/SnpDecoderOpenH264.cpp src/stream/video/SnpDecoderOpenH264.h)
endif(LIBGL_FOUND)

if(LIBWEBSOCKETS_FOUND)
    set(WEBSOCKETS_SOURCES
        src/network/SnpClientWebsocket.cpp
        src/network/SnpWebsocket.cpp
        src/stream/network/SnpSinkNetworkWebsocket.cpp
    )
endif(LIBWEBSOCKETS_FOUND)

###############################################################################
# compile protobuf
###############################################################################
add_custom_command(
        OUTPUT snappy-protobuf-sources
        COMMAND
            "${CMAKE_CURRENT_SOURCE_DIR}/extern/protobuf-3.21.12/cmake-build-release/protoc.exe"
            "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto"
            "--cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src/network"
            "${CMAKE_CURRENT_SOURCE_DIR}/proto/snp.proto"
)
add_custom_target(${PROTO_TARGET_NAME} DEPENDS snappy-protobuf-sources)

###############################################################################
# client app
###############################################################################
add_executable(snpserver
    src/snpserver.cpp
    src/network/snp.pb.cc
    src/config/SnpConfig.cpp
    src/util/loguru.cpp
    src/util/TimeUtil.cpp
    src/util/VideoUtil.cpp
    src/util/PropertyUtil.cpp
    # stream
    src/stream/SnpComponentRegistry.cpp
    src/stream/SnpProperty.cpp
    src/stream/SnpComponent.cpp
    src/stream/SnpPort.cpp
    src/stream/SnpPipe.cpp
    src/stream/SnpPipeFactory.cpp
    # video stream
    src/stream/video/SnpSourceDummy.cpp
    src/stream/output/SnpSinkDisplay.cpp
    # input stream
    src/stream/input/keymap_atset1_linux.c
    # network sink/source
    src/stream/network/SnpSourceNetworkTcp.cpp
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
   set(SOCKET_LIBRARIES wsock32 ws2_32)
endif(WIN32)

# server app
target_link_libraries(snpserver
    ${OPENH264_LIBRARIES}
    ${SOCKET_LIBRARIES}
    pthread
    libprotobuf
    #dl
)

###############################################################################
# client app
###############################################################################
add_executable(snpclient
    src/snpclient.cpp
    # gui
    src/gui/SnpCanvas.cpp
    # general
    src/network/snp.pb.cc
    src/config/SnpConfig.cpp
    src/util/loguru.cpp
    src/util/TimeUtil.cpp
    src/util/VideoUtil.cpp
    src/util/PropertyUtil.cpp
    # stream
    src/stream/SnpComponentRegistry.cpp
    src/stream/SnpProperty.cpp
    src/stream/SnpComponent.cpp
    src/stream/SnpPort.cpp
    src/stream/SnpPipe.cpp
    src/stream/SnpPipeFactory.cpp
    # video stream
    src/stream/video/SnpSourceDummy.cpp
    src/stream/output/SnpSinkDisplay.cpp
    # input stream
    src/stream/input/keymap_atset1_linux.c
    # network sink/source
    src/stream/network/SnpSourceNetworkTcp.cpp
    src/stream/network/SnpSinkNetworkTcp.cpp
    # file sink/source
    src/stream/file/SnpSinkFile.cpp
    ${X11_SOURCES}
    ${DRM_SOURCES}
    ${GL_SOURCES}
    ${MMALH264_SOURCES}
    ${OPENH264_SOURCES}
    ${VA_SOURCES})
target_link_libraries(snpclient
    Qt::Core
    Qt::Gui
    Qt::Widgets
    ${OPENH264_LIBRARIES}
    ${SOCKET_LIBRARIES}
    pthread
    libprotobuf
)

if (WIN32)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${CLIENT_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${CLIENT_TARGET_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${CLIENT_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${CLIENT_TARGET_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${CLIENT_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${CLIENT_TARGET_NAME}>")
    endforeach (QT_LIB)
endif ()