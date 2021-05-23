# Find MMALH264 libraries
# MMALH264_FOUND - system has the userland libraries
# MMALH264_INCLUDE_DIR - the userland libraries include dir
# MMALH264_LIBRARIES - the userland libraries needed to use MMALH264

if (MMALH264_INCLUDE_DIR AND MMALH264_LIBRARIES)
    # in cache already
    SET(MMALH264_FOUND TRUE)
else (MMALH264_INCLUDE_DIR AND MMALH264_LIBRARIES)

    set(MMALH264_INCLUDE_DIR /opt/vc/include /opt/vc/include/interface/mmal)
    set(MMALH264_LIBRARY_DIR /opt/vc/lib)

    message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
        set(X86_64 1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*|amd64.*|AMD64.*")
        set(X86 1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*)")
        set(AARCH64 1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
        set(ARM 1)
    else()
        message(STATUS "Processor is not supported!")
    endif()

    if(ARM)
        find_path(MMAL_H
                NAMES "mmal.h"
                PATHS ${MMALH264_INCLUDE_DIR})

        find_library(LIB_MMAL
                NAMES libmmal.so
                PATHS ${MMALH264_LIBRARY_DIR})

        find_library(LIB_MMAL_CORE
                NAMES libmmal_core.so
                PATHS ${MMALH264_LIBRARY_DIR})

        find_library(LIB_MMAL_UTIL
                NAMES libmmal_util.so
                PATHS ${MMALH264_LIBRARY_DIR})

        find_library(LIB_MMAL_COMPONENTS
                NAMES libmmal_components.so
                PATHS ${MMALH264_LIBRARY_DIR})

        find_library(LIB_VCOS
                NAMES libvcos.so
                PATHS ${MMALH264_LIBRARY_DIR})

        find_library(LIB_VC_CLIENT
                NAMES libmmal_vc_client.so
                PATHS ${MMALH264_LIBRARY_DIR})

        find_library(LIB_VCSM
                NAMES libvcsm.so
                PATHS ${MMALH264_LIBRARY_DIR})
    elseif(AARCH64)
        message(STATUS "Architecture is not supported!")
        #FIND_LIBRARY(MMALH264_LIBRARIES
        #        NAMES libMMALh264-2.1.1-android-arm64.so
        #        PATHS ${MMALH264_LIBRARY_DIR})
    endif()

    set(MMALH264_LIBRARIES
            ${LIB_MMAL}
            ${LIB_MMAL_CORE}
            ${LIB_MMAL_UTIL}
            ${LIB_MMAL_COMPONENTS}
            ${LIB_VCOS}
            ${LIB_VC_CLIENT}
            ${LIB_VCSM}
            )

    if (MMALH264_INCLUDE_DIR AND MMALH264_LIBRARIES)
        set(MMALH264_FOUND TRUE)
    endif (MMALH264_INCLUDE_DIR AND MMALH264_LIBRARIES)

    if (MMALH264_FOUND)
        if (NOT MMALH264_FIND_QUIETLY)
            message(STATUS "Found MMAL libraries: ${MMALH264_LIBRARIES}")
        endif (NOT MMALH264_FIND_QUIETLY)
    else (MMALH264_FOUND)
        if (MMALH264_FIND_REQUIRED)
            message(FATAL_ERROR "Could NOT find MMALH264")
        else()
            message(STATUS "Could NOT find MMALH264")
        endif (MMALH264_FIND_REQUIRED)
    endif (MMALH264_FOUND)

    #	MARK_AS_ADVANCED(MMALH264_INCLUDE_DIR MMALH264_LIBRARIES)
endif (MMALH264_INCLUDE_DIR AND MMALH264_LIBRARIES)
