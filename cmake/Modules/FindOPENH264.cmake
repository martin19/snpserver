# Find libOPENH264
# OPENH264_FOUND - system has the OPENH264 library
# OPENH264_INCLUDE_DIR - the OPENH264 include directory
# OPENH264_LIBRARIES - The libraries needed to use OPENH264

if (OPENH264_INCLUDE_DIR AND OPENH264_LIBRARIES)
    # in cache already
    SET(OPENH264_FOUND TRUE)
else (OPENH264_INCLUDE_DIR AND OPENH264_LIBRARIES)

    set(OPENH264_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/openh264/include)
    set(OPENH264_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/openh264/lib)

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

    if(ANDROID_ABI MATCHES "armeabi-v7a")
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-android-arm.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(ANDROID_ABI MATCHES "arm64-v8a")
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-android-arm64.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(ANDROID_ABI MATCHES "x86")
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-android-x86.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(ANDROID_ABI MATCHES "x86_64")
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-android-x64.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(UNIX AND X86)
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-linux32.6.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(UNIX AND X86_64)
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-linux64.6.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(UNIX AND ARM)
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-linux-arm.6.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(UNIX AND AARCH64)
        FIND_LIBRARY(OPENH264_LIBRARIES
                NAMES libopenh264-2.1.1-linux-arm64.6.so
                PATHS ${OPENH264_LIBRARY_DIR})
    elseif(WIN32 AND X86)
        set(OPENH264_LIBRARIES "")
    elseif(WIN32 AND X86_64)
        set(OPENH264_LIBRARIES "")
    endif()

    if (OPENH264_INCLUDE_DIR AND OPENH264_LIBRARIES)
        set(OPENH264_FOUND TRUE)
    elseif(WIN32)
        set(OPENH264_FOUND TRUE)
    endif (OPENH264_INCLUDE_DIR AND OPENH264_LIBRARIES)

    if (OPENH264_FOUND)
        if (NOT OPENH264_FIND_QUIETLY)
            message(STATUS "Found OPENH264: ${OPENH264_LIBRARIES}")
        endif (NOT OPENH264_FIND_QUIETLY)
    else (OPENH264_FOUND)
        if (OPENH264_FIND_REQUIRED)
            message(FATAL_ERROR "Could NOT find OPENH264")
        else()
            message(STATUS "Could NOT find OPENH264")
        endif (OPENH264_FIND_REQUIRED)
    endif (OPENH264_FOUND)
endif (OPENH264_INCLUDE_DIR AND OPENH264_LIBRARIES)
