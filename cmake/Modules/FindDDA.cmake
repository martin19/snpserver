# FindDDA.cmake
# Locate the Desktop Duplication API (based on Direct3D 11 + DXGI1.2)
#
# This will define:
#   DDA_FOUND        - True if headers and libs were found
#   DDA_INCLUDE_DIRS - Paths to required headers
#   DDA_LIBRARIES    - Libraries to link against (d3d11, dxgi)
#   DDA::DDA         - An imported target for modern usage
#
# Example:
#   find_package(DDA)
#   if(DDA_FOUND)
#       target_link_libraries(myapp PRIVATE DDA::DDA)
#   endif()

include(CheckIncludeFileCXX)

# Check for required headers
check_include_file_cxx(d3d11.h HAVE_D3D11_H)
check_include_file_cxx(dxgi1_2.h HAVE_DXGI1_2_H)

if(HAVE_D3D11_H AND HAVE_DXGI1_2_H)
    set(DDA_FOUND TRUE)

    # On Windows these headers are provided by the Windows SDK,
    # so no extra include dirs are usually needed.
    set(DDA_INCLUDE_DIRS "")

    # Required libraries for Desktop Duplication
    set(DDA_LIBRARIES d3d11 dxgi)

    if(NOT TARGET DDA::DDA)
        add_library(DDA::DDA INTERFACE IMPORTED)
        set_target_properties(DDA::DDA PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${DDA_INCLUDE_DIRS}"
                INTERFACE_LINK_LIBRARIES "${DDA_LIBRARIES}"
                )
    endif()

else()
    set(DDA_FOUND FALSE)
endif()