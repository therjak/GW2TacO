set(PROJECT_NAME third_party)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "jsonxx.h"
    "miniz.h"
    "RapidXML/rapidxml.hpp"
    "RapidXML/rapidxml_iterators.hpp"
    "RapidXML/rapidxml_print.hpp"
    "RapidXML/rapidxml_utils.hpp"
    "stb_image.h"
    "stb_image_write.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "jsonxx.cc"
    "miniz.c"
)
source_group("Source Files" FILES ${Source_Files})

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} 
    PRIVATE 
        ${Source_Files}
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/../..;"
    FILES ${Header_Files}
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE UtilLib)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "win32")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG_MULTIBYTE   "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
        OUTPUT_DIRECTORY_DEBUG             "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASE_MULTIBYTE "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASE           "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE_MULTIBYTE "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_RELEASE           "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Release_MultiByte>:
        MultiThreaded
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:Release_MultiByte>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug_MultiByte>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release_MultiByte>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "NOMINMAX;"
    "_CRT_SECURE_NO_WARNINGS;"
    "WIN32_LEAN_AND_MEAN;"
    "WIN32;"
    "_LIB;"
    "_MBCS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug_MultiByte>:
            /Gm-;
            /Od
        >
        $<$<CONFIG:Debug>:
            /Gm-;
            /Od
        >
        $<$<CONFIG:Release_MultiByte>:
            /O2;
            /Oi;
            /Gy
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi;
            /Gy
        >
        /MP;
        /std:c++latest;
        /W3;
        /MD;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        /utf-8;
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        ${DEFAULT_CXX_RUNTIME_LIBRARY};
        /Y-
        )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release_MultiByte>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

