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

# library: QT 6.9.1+
set(QT_VERSION 6.9.1)
set(Qt6_DIR C:/Qt/${QT_VERSION}/mingw_64/lib/cmake/Qt6/)
set(CMAKE_PREFIX_PATH C:/Qt/${QT_VERSION}/mingw_64/lib/cmake)
find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    REQUIRED
)

# ffmpeg 7.1.1
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/extern/ffmpeg")
set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIR}/include")
set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/lib")
set(FFMPEG_LIBRARIES
    ${FFMPEG_LIB_DIR}/libavcodec.dll.a
    ${FFMPEG_LIB_DIR}/libavutil.dll.a
    ${FFMPEG_LIB_DIR}/libswscale.dll.a
    ${FFMPEG_LIB_DIR}/libavformat.dll.a
    ${FFMPEG_LIB_DIR}/libavdevice.dll.a
)
link_directories(${FFMPEG_LIB_DIR})
include_directories(${FFMPEG_INCLUDE_DIR})

find_package(LibWebSockets)
if(LIBWEBSOCKETS_FOUND)
    add_definitions(-DHAVE_LIBWEBSOCKETS)
endif(LIBWEBSOCKETS_FOUND)

include_directories(
    src
    extern/openh264/include
    ${FFMPEG_INCLUDE_DIR}
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
    add_library(libdrm SHARED IMPORTED src/stream/output/SnpSinkDisplay.cpp src/stream/output/SnpSinkDisplay.h src/stream/video/va/DXUtil.cpp src/stream/video/va/DXUtil.h)
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
    )
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
# server app
###############################################################################
add_executable(snpserver
    src/snpserver.cpp
    src/network/snp.pb.cc
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
    src/stream/video/SnpSourceDda.cpp
    src/stream/output/SnpSinkDisplay.cpp
    src/stream/video/SnpEncoderFFmpeg.cpp
    src/stream/video/SnpDecoderFFmpeg.cpp
    # input stream
    src/stream/input/keymap_atset1_linux.c
    # network sink/source
    src/stream/network/SnpSinkNetworkTcp.cpp
    # file sink/source
    src/stream/file/SnpSinkFile.cpp
    # data
    src/stream/data/SnpData.cpp
    src/stream/data/SnpDataDx11.cpp
    src/stream/data/SnpDataRam.cpp
    # context
    src/context/SnpContext.cpp
    # system dependent
    ${X11_SOURCES}
    ${DRM_SOURCES}
    ${GL_SOURCES}
)

target_compile_options(
    snpserver
    PUBLIC
    $<$<CONFIG:Debug>:-g>        # Add debugging symbols in Debug mode
    $<$<CONFIG:Release>:-O3>     # Add optimization in Release mode
)

if(LIBWEBSOCKETS_FOUND)
    target_link_libraries(snpserver ${LIBWEBSOCKETS_LIBRARIES})
endif(LIBWEBSOCKETS_FOUND)

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
   set(WIN32_LIBRARIES winmm d3d11 dxgi dxguid uuid)
endif(WIN32)

# server app
target_link_libraries(snpserver
    ${SOCKET_LIBRARIES}
    ${WIN32_LIBRARIES}
    pthread
    libprotobuf
    ${FFMPEG_LIBRARIES}
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
    src/stream/video/SnpSourceDda.cpp
    src/stream/output/SnpSinkDisplay.cpp
    src/stream/video/SnpEncoderFFmpeg.cpp
    src/stream/video/SnpDecoderFFmpeg.cpp
    # input stream
    src/stream/input/keymap_atset1_linux.c
    # network sink/source
    src/stream/network/SnpSourceNetworkTcp.cpp
    src/stream/network/SnpSinkNetworkTcp.cpp
    # file sink/source
    src/stream/file/SnpSinkFile.cpp
    # data
    src/stream/data/SnpData.cpp
    src/stream/data/SnpDataDx11.cpp
    src/stream/data/SnpDataRam.cpp
    # context
    src/context/SnpContext.cpp
    ${X11_SOURCES}
    ${DRM_SOURCES}
    ${GL_SOURCES}
)
target_link_libraries(snpclient
    Qt::Core
    Qt::Gui
    Qt::Widgets
    ${SOCKET_LIBRARIES}
    ${WIN32_LIBRARIES}
    pthread
    libprotobuf
    ${FFMPEG_LIBRARIES}
)

target_compile_options(
        snpclient
        PUBLIC
        $<$<CONFIG:Debug>:-g>        # Add debugging symbols in Debug mode
        $<$<CONFIG:Release>:-O3>     # Add optimization in Release mode
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
