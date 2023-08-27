cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(GW2TacO C CXX)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "win32"
     OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Debug_MultiByte"
    "Release"
    "Release_MultiByte"
    CACHE STRING "" FORCE
)


################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG_MULTIBYTE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG_MULTIBYTE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_MULTIBYTE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG_MULTIBYTE "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE_MULTIBYTE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE_MULTIBYTE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_MULTIBYTE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE_MULTIBYTE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################
add_subdirectory(src/util)

set(PROJECT_NAME GW2TacO)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "src/base/enum_helpers.h"
    "src/base/image_decompressor.h"
    "src/core2/core2.h"
    "src/core2/core2_config.h"
    "src/core2/dss_texture_loader.h"
    "src/core2/texture.h"
    "src/dungeon_progress.h"
    "src/gw2_api.h"
    "src/gw2_taco.h"
    "src/gw2_tactical.h"
    "src/hp_grid.h"
    "src/language.h"
    "src/locational_timer.h"
    "src/map_timer.h"
    "src/marker_editor.h"
    "src/mouse_highlight.h"
    "src/mumble_link.h"
    "src/notepad.h"
    "src/overlay_application.h"
    "src/overlay_config.h"
    "src/overlay_window.h"
    "src/raid_progress.h"
    "src/range_display.h"
    "src/resource.h"
    "src/special_gui_items.h"
    "src/tactical_compass.h"
    "src/tp_tracker.h"
    "src/trail_logger.h"
    "src/ts3_connection.h"
    "src/ts3_control.h"
    "src/white_board/application.h"
    "src/white_board/atlas.h"
    "src/white_board/css_item.h"
    "src/white_board/draw_api.h"
    "src/white_board/message.h"
    "src/white_board/skin.h"
    "src/white_board/style_manager.h"
    "src/wvw.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Header_Files__DX
    "src/core2/dx11_constant_buffer.h"
    "src/core2/dx11_device.h"
    "src/core2/dx11_enums.h"
    "src/core2/dx11_index_buffer.h"
    "src/core2/dx11_render_state.h"
    "src/core2/dx11_shader.h"
    "src/core2/dx11_texture.h"
    "src/core2/dx11_vertex_buffer.h"
    "src/core2/dx11_vertex_format.h"
)
source_group("Header Files\\DX" FILES ${Header_Files__DX})

set(Header_Files__GuiItems
    "src/white_board/box.h"
    "src/white_board/button.h"
    "src/white_board/context_menu.h"
    "src/white_board/gui_item.h"
    "src/white_board/label.h"
    "src/white_board/root.h"
    "src/white_board/text_box.h"
    "src/white_board/window.h"
)
source_group("Header Files\\GuiItems" FILES ${Header_Files__GuiItems})

set(Header_Files__System
    "src/base/hasher.h"
    "src/base/socket.h"
    "src/base/timer.h"
    "src/core2/constant_buffer.h"
    "src/core2/device.h"
    "src/core2/enums.h"
    "src/core2/index_buffer.h"
    "src/core2/render_state.h"
    "src/core2/resource.h"
    "src/core2/shader.h"
    "src/core2/vertex_buffer.h"
    "src/core2/vertex_format.h"
    "src/core2/window_handler.h"
    "src/white_board/font.h"
)
source_group("Header Files\\System" FILES ${Header_Files__System})

set(Header_Files__base
    "src/base/assert.h"
    "src/base/logger.h"
    "src/build_count.h"
    "src/build_info.h"
)
source_group("Header Files\\base" FILES ${Header_Files__base})

set(Header_Files__math
    "src/base/matrix.h"
    "src/base/plane.h"
    "src/base/rectangle.h"
    "src/base/spec_math.h"
    "src/base/sphere.h"
    "src/base/vector.h"
)
source_group("Header Files\\math" FILES ${Header_Files__math})

set(Header_Files__util
    "src/base/color.h"
    "src/base/file_list.h"
    "src/base/ring_buffer.h"
    "src/base/stream_reader.h"
    "src/base/stream_writer.h"
    "src/base/string_format.h"
    "src/pro_font.h"
    "src/util/png_decompressor.h"
    "src/util/xml_document.h"
    "src/util/xml_node.h"
)
source_group("Header Files\\util" FILES ${Header_Files__util})

set(Resources
    "maptimer.xml"
    "notepad.txt"
    "TacO_Language_en.xml"
    "UI.css"
    "UI.xml"
)
source_group("Resources" FILES ${Resources})

set(Source_Files
    "src/core2/constant_buffer.cpp"
    "src/core2/device.cpp"
    "src/core2/dss_texture_loader.cpp"
    "src/core2/render_state.cpp"
    "src/core2/resource.cpp"
    "src/core2/shader.cpp"
    "src/core2/window_handler.cpp"
    "src/dungeon_progress.cpp"
    "src/gw2_api.cpp"
    "src/gw2_pois.cpp"
    "src/gw2_taco.cpp"
    "src/gw2_tactical.cpp"
    "src/hp_grid.cpp"
    "src/language.cpp"
    "src/locational_timer.cpp"
    "src/map_timer.cpp"
    "src/marker_editor.cpp"
    "src/mouse_highlight.cpp"
    "src/mumble_link.cpp"
    "src/notepad.cpp"
    "src/overlay_application.cpp"
    "src/overlay_config.cpp"
    "src/overlay_window.cpp"
    "src/raid_progress.cpp"
    "src/range_display.cpp"
    "src/special_gui_items.cpp"
    "src/tactical_compass.cpp"
    "src/tp_tracker.cpp"
    "src/trail_logger.cpp"
    "src/ts3_connection.cpp"
    "src/ts3_control.cpp"
    "src/white_board/application.cpp"
    "src/white_board/atlas.cpp"
    "src/white_board/box.cpp"
    "src/white_board/button.cpp"
    "src/white_board/context_menu.cpp"
    "src/white_board/css_item.cpp"
    "src/white_board/draw_api.cpp"
    "src/white_board/gui_item.cpp"
    "src/white_board/label.cpp"
    "src/white_board/message.cpp"
    "src/white_board/root.cpp"
    "src/white_board/skin.cpp"
    "src/white_board/style_manager.cpp"
    "src/white_board/text_box.cpp"
    "src/white_board/window.cpp"
    "src/wvw.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(Source_Files__base
    "src/base/logger.cpp"
    "src/build_info.cpp"
)
source_group("Source Files\\base" FILES ${Source_Files__base})

set(Source_Files__dx
    "src/core2/dx11_constant_buffer.cpp"
    "src/core2/dx11_device.cpp"
    "src/core2/dx11_enums.cpp"
    "src/core2/dx11_index_buffer.cpp"
    "src/core2/dx11_render_state.cpp"
    "src/core2/dx11_shader.cpp"
    "src/core2/dx11_texture.cpp"
    "src/core2/dx11_vertex_buffer.cpp"
    "src/core2/dx11_vertex_format.cpp"
)
source_group("Source Files\\dx" FILES ${Source_Files__dx})

set(Source_Files__math
    "src/base/matrix.cpp"
    "src/base/spec_math.cpp"
    "src/base/vector.cpp"
)
source_group("Source Files\\math" FILES ${Source_Files__math})

set(Source_Files__util
    "src/base/file_list.cpp"
    "src/base/hasher.cpp"
    "src/base/image_decompressor.cpp"
    "src/base/socket.cpp"
    "src/base/stream_reader.cpp"
    "src/base/stream_writer.cpp"
    "src/base/string_format.cpp"
    "src/base/timer.cpp"
    "src/pro_font.cpp"
    "src/util/png_decompressor.cpp"
    "src/util/xml_document.cpp"
    "src/util/xml_node.cpp"
    "src/white_board/font.cpp"
)
source_group("Source Files\\util" FILES ${Source_Files__util})

set(ALL_FILES
    ${Header_Files}
    ${Header_Files__DX}
    ${Header_Files__GuiItems}
    ${Header_Files__System}
    ${Header_Files__base}
    ${Header_Files__math}
    ${Header_Files__util}
    ${Resources}
    ${Source_Files}
    ${Source_Files__base}
    ${Source_Files__dx}
    ${Source_Files__math}
    ${Source_Files__util}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE GW2Overlay)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)

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
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
"${CMAKE_CURRENT_SOURCE_DIR}/."
)

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
"_WINDOWS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug_MultiByte>:
                /Od
            >
            $<$<CONFIG:Debug>:
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
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /utf-8;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug_MultiByte>:
                /INCREMENTAL
            >
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release_MultiByte>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:WINDOWS
        )
endif()

################################################################################
# Pre build events
################################################################################


################################################################################
# Dependencies
################################################################################
# Link with other targets.
target_link_libraries(${PROJECT_NAME} PRIVATE
    third_party
)

