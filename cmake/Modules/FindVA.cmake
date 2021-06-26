# Find libVA
# VA_FOUND - system has the VA library
# VA_INCLUDE_DIR - the VA include directory
# VA_LIBRARIES - The libraries needed to use VA

if (VA_INCLUDE_DIR AND VA_LIBRARIES)
    # in cache already
    SET(VA_FOUND TRUE)
else (VA_INCLUDE_DIR AND VA_LIBRARIES)

    find_path(VA_INCLUDE_DIR va/va.h REQUIRED)
    find_library(VA_LIBRARY NAMES va REQUIRED)
    find_library(VA_DRM_LIBRARY NAMES va-drm REQUIRED)
    find_library(VA_X11_LIBRARY NAMES va-x11 REQUIRED)

    set(VA_LIBRARIES ${VA_LIBRARY}
                     ${VA_DRM_LIBRARY}
                     ${VA_X11_LIBRARY}
    )

    if (VA_INCLUDE_DIR AND VA_LIBRARIES)
        set(VA_FOUND TRUE)
    endif (VA_INCLUDE_DIR AND VA_LIBRARIES)

    if (VA_FOUND)
        if (NOT VA_FIND_QUIETLY)
            message(STATUS "Found VA: ${VA_LIBRARIES}")
        endif (NOT VA_FIND_QUIETLY)
    else (VA_FOUND)
        if (VA_FIND_REQUIRED)
            message(FATAL_ERROR "Could NOT find VA")
        else()
            message(STATUS "Could NOT find VA")
        endif (VA_FIND_REQUIRED)
    endif (VA_FOUND)

    #	MARK_AS_ADVANCED(VA_INCLUDE_DIR VA_LIBRARIES)
endif (VA_INCLUDE_DIR AND VA_LIBRARIES)
