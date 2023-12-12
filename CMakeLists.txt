cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(GW2TacO C CXX)
set(CMAKE_CXX_STANDARD 20)

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
    "src/base/assert.h"
    "src/base/color.h"
    "src/base/enum_helpers.h"
    "src/base/file_list.h"
    "src/base/hasher.h"
    "src/base/image_decompressor.h"
    "src/base/logger.h"
    "src/base/matrix.h"
    "src/base/plane.h"
    "src/base/rectangle.h"
    "src/base/ring_buffer.h"
    "src/base/socket.h"
    "src/base/spec_math.h"
    "src/base/sphere.h"
    "src/base/stream_reader.h"
    "src/base/stream_writer.h"
    "src/base/string_format.h"
    "src/base/timer.h"
    "src/base/vector.h"
    "src/build_info.h"
    "src/core2/constant_buffer.h"
    "src/core2/core2.h"
    "src/core2/core2_config.h"
    "src/core2/device.h"
    "src/core2/dss_texture_loader.h"
    "src/core2/dx11_constant_buffer.h"
    "src/core2/dx11_device.h"
    "src/core2/dx11_enums.h"
    "src/core2/dx11_index_buffer.h"
    "src/core2/dx11_render_state.h"
    "src/core2/dx11_shader.h"
    "src/core2/dx11_texture.h"
    "src/core2/dx11_vertex_buffer.h"
    "src/core2/dx11_vertex_format.h"
    "src/core2/enums.h"
    "src/core2/index_buffer.h"
    "src/core2/render_state.h"
    "src/core2/resource.h"
    "src/core2/shader.h"
    "src/core2/texture.h"
    "src/core2/vertex_buffer.h"
    "src/core2/vertex_format.h"
    "src/core2/window_handler.h"
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
    "src/pro_font.h"
    "src/raid_progress.h"
    "src/range_display.h"
    "src/resource.h"
    "src/special_gui_items.h"
    "src/tactical_compass.h"
    "src/tp_tracker.h"
    "src/trail_logger.h"
    "src/ts3_connection.h"
    "src/ts3_control.h"
    "src/util/png_decompressor.h"
    "src/util/xml_document.h"
    "src/util/xml_node.h"
    "src/white_board/application.h"
    "src/white_board/atlas.h"
    "src/white_board/box.h"
    "src/white_board/button.h"
    "src/white_board/context_menu.h"
    "src/white_board/css_item.h"
    "src/white_board/draw_api.h"
    "src/white_board/font.h"
    "src/white_board/gui_item.h"
    "src/white_board/label.h"
    "src/white_board/message.h"
    "src/white_board/root.h"
    "src/white_board/skin.h"
    "src/white_board/style_manager.h"
    "src/white_board/text_box.h"
    "src/white_board/window.h"
    "src/wvw.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Resources
    "maptimer.xml"
    "notepad.txt"
    "hpgrids.xml"
    "locationaltimers.xml"
    "maptimer.xml"
    "TacO_Language_en.xml"
    "UI.css"
    "UI_large.css"
    "UI_larger.css"
    "UI_small.css"
    "UI_normal.css"
    "UI.wbs"
    "UI.xml"
)
source_group("Resources" FILES ${Resources})

set(Source_Files
    "src/base/file_list.cpp"
    "src/base/hasher.cpp"
    "src/base/image_decompressor.cpp"
    "src/base/logger.cpp"
    "src/base/matrix.cpp"
    "src/base/socket.cpp"
    "src/base/spec_math.cpp"
    "src/base/stream_reader.cpp"
    "src/base/stream_writer.cpp"
    "src/base/string_format.cpp"
    "src/base/timer.cpp"
    "src/base/vector.cpp"
    "src/build_info.cpp"
    "src/core2/constant_buffer.cpp"
    "src/core2/device.cpp"
    "src/core2/dss_texture_loader.cpp"
    "src/core2/dx11_constant_buffer.cpp"
    "src/core2/dx11_device.cpp"
    "src/core2/dx11_enums.cpp"
    "src/core2/dx11_index_buffer.cpp"
    "src/core2/dx11_render_state.cpp"
    "src/core2/dx11_shader.cpp"
    "src/core2/dx11_texture.cpp"
    "src/core2/dx11_vertex_buffer.cpp"
    "src/core2/dx11_vertex_format.cpp"
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
    "src/pro_font.cpp"
    "src/raid_progress.cpp"
    "src/range_display.cpp"
    "src/special_gui_items.cpp"
    "src/tactical_compass.cpp"
    "src/tp_tracker.cpp"
    "src/trail_logger.cpp"
    "src/ts3_connection.cpp"
    "src/ts3_control.cpp"
    "src/util/png_decompressor.cpp"
    "src/util/xml_document.cpp"
    "src/util/xml_node.cpp"
    "src/white_board/application.cpp"
    "src/white_board/atlas.cpp"
    "src/white_board/box.cpp"
    "src/white_board/button.cpp"
    "src/white_board/context_menu.cpp"
    "src/white_board/css_item.cpp"
    "src/white_board/draw_api.cpp"
    "src/white_board/font.cpp"
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

set(Data_Files
    "Data/Achievement.png"
    "Data/airshippart.png"
    "Data/aurillium.png"
    "Data/bigmarker.png"
    "Data/Bounty.png"
    "Data/Bounty_bw.png"
    "Data/Chest.png"
    "Data/crystallineore.png"
    "Data/east.png"
    "Data/empty.png"
    "Data/four.png"
    "Data/gem.png"
    "Data/Gold_coin.png"
    "Data/leylinecrystal.png"
    "Data/north.png"
    "Data/northeast.png"
    "Data/northwest.png"
    "Data/one.png"
    "Data/Ore.png"
    "Data/Plant.png"
    "Data/south.png"
    "Data/southeast.png"
    "Data/southwest.png"
    "Data/spawn.png"
    "Data/stack.png"
    "Data/storyline_green.png"
    "Data/storyline_purple.png"
    "Data/three.png"
    "Data/two.png"
    "Data/west.png"
    "Data/Wood.png"    
)
source_group("Data Files" FILES ${Data_Files})

set(Poi_Files
    "POIs/collectibles_astudyingold.xml"
    "POIs/collectibles_goldenlostbadges.xml"
    "POIs/firecracker.xml"
    "POIs/GuildMission_Bounty_2Mult.xml"
    "POIs/GuildMission_Bounty_BigMayana.xml"
    "POIs/GuildMission_Bounty_BookwormBwikki.xml"
    "POIs/GuildMission_Bounty_Brekkabek.xml"
    "POIs/GuildMission_Bounty_Crusader_Michiele.xml"
    "POIs/GuildMission_Bounty_Deputy_Brooke.xml"
    "POIs/GuildMission_Bounty_Devious_Teesa.xml"
    "POIs/GuildMission_Bounty_Diplomat_Tarban.xml"
    "POIs/GuildMission_Bounty_Half_Baked_Komali.xml"
    "POIs/GuildMission_Bounty_Poobadoo.xml"
    "POIs/GuildMission_Bounty_Prisoner1141.xml"
    "POIs/GuildMission_Bounty_Shaman_Arderus.xml"
    "POIs/GuildMission_Bounty_Short_Fuse_Felix.xml"
    "POIs/GuildMission_Bounty_Sotzz.xml"
    "POIs/GuildMission_Bounty_Tricksy_Trekksa.xml"
    "POIs/GuildMission_Bounty_Trillia_Midwell.xml"
    "POIs/GuildMission_Bounty_Yanonka.xml"
    "POIs/keyrequired_dragonsstand.xml"
    "POIs/keyrequired_drytop.xml"
    "POIs/keyrequired_silverwastes.xml"
    "POIs/keyrequired_tangleddepths.xml"
    "POIs/keyrequired_verdantbrink.xml"
    "POIs/raid1_closure_achievement.xml"
    "POIs/raid1_piecing_it_together_achievement.xml"
    "POIs/raid1_salvationpass_markers.xml"
    "POIs/raid1_scattered_memories.xml"
    "POIs/raid1_spiritvale_markers.xml"
    "POIs/raid1_stronghold_markers.xml"
    "POIs/SAB_Bombs.xml"
    "POIs/SAB_Dig_Locations.xml"
    "POIs/SAB_EnvironmentalScience.xml"
    "POIs/SAB_FluteDoor.xml"
    "POIs/SAB_FurnitureShops.xml"
    "POIs/SAB_Shops.xml"
    "POIs/SAB_Tribulation.xml"
    "POIs/wintersday_orphan_locations.xml"
)
source_group("Poi Files" FILES ${Poi_Files})

set(Poi_Data_Files
    "POIs/Data/Auric_Basin.png"
    "POIs/Data/Bomb.png"
    "POIs/Data/Herta.png"
    "POIs/Data/Karma.png"
    "POIs/Data/Lunar_New_Year_Dailies.png"   
    "POIs/Data/Note.png"
    "POIs/Data/raid1_achievements.png"       
    "POIs/Data/SAB_achievement.png"
    "POIs/Data/The_Silverwastes.png"
)
source_group("Poi Files\\Data" FILES ${Poi_Data_Files})


set(ALL_FILES
    ${Header_Files}
    ${Resources}
    ${Source_Files}
    ${Data_Files}
    ${Poi_Files}
    ${Poi_Data_Files}
)

# Add version information
find_package(Git)
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT GIT_DESCRIBE_ERROR_CODE)
    set(TACO_VERSION ${GIT_DESCRIBE_VERSION})
  endif()
endif()

if(NOT DEFINED TACO_VERSION)
  set(TACO_VERSION v0.0.0-unknown)
  message(WARNING "Failed to determine TACO_VERSION from Git tags. Using default version \"${TACO_VERSION}\".")
endif()

configure_file(src/build_count.h.in src/build_count.h @ONLY)

include_directories(${CMAKE_BINARY_DIR})

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        ${Source_Files}
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/."    
    FILES ${Header_Files}
)

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
            /MD;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /utf-8;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            ${DEFAULT_CXX_RUNTIME_LIBRARY};
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


install(
    TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "."
)
install(
    FILES ${Resources} DESTINATION "."
)
install(
    FILES ${Data_Files} DESTINATION "./Data"
)
install(
    FILES ${Poi_Files} DESTINATION "./POIs"
)
install(
    FILES ${Poi_Data_Files} DESTINATION "./POIs/Data"
)

################################################################################
# CPACK
################################################################################

set(CPACK_GENERATOR "ZIP")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${TACO_VERSION})
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
include(CPack)
