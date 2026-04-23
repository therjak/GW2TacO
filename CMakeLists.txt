cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
project(GW2TacO LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "$<$<OR:$<CONFIG:Release>,$<CONFIG:Release_MultiByte>>:MultiThreaded>$<$<NOT:$<OR:$<CONFIG:Release>,$<CONFIG:Release_MultiByte>>>:MultiThreaded$<$<CONFIG:Debug,Debug_MultiByte>:Debug>DLL>")
if(MSVC)
    add_compile_options(/utf-8 /MP /W3)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
)
# For Windows: Do not override the settings
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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
include(CMake/CommonDefinitions.cmake)


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
enable_testing()

set(PROJECT_NAME GW2TacO)

################################################################################
# Source groups
################################################################################
set(Header_Files
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
    "src/gw2_tactical.h"
    "src/resource.h"
    "src/util/png_decompressor.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Base_Source_Files
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
)

set(Core2_Source_Files
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
)

set(Util_Source_Files
    "src/util/png_decompressor.cpp"
    "src/util/xml_document.cpp"
    "src/util/xml_node.cpp"
)

set(Whiteboard_Source_Files
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
)

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

set(Taco_Source_Files
    "src/build_info.cpp"
    "src/gw2_taco.cpp"
    "src/gw2_tactical.cpp"
    "src/dungeon_progress.cpp"
    "src/gw2_api.cpp"
    "src/hp_grid.cpp"
    "src/language.cpp"
    "src/locational_timer.cpp"
    "src/map_timer.cpp"
    "src/marker_data.cpp"
    "src/marker_editor.cpp"
    "src/mouse_highlight.cpp"
    "src/mumble_link.cpp"
    "src/notepad.cpp"
    "src/overlay_application.cpp"
    "src/overlay_config.cpp"
    "src/overlay_window.cpp"
    "src/string_set.cpp"
    "src/pro_font.cpp"
    "src/raid_progress.cpp"
    "src/range_display.cpp"
    "src/special_gui_items.cpp"
    "src/tactical_compass.cpp"
    "src/tactical_category.cpp"
    "src/tp_tracker.cpp"
    "src/trail_logger.cpp"
    "src/ts3_connection.cpp"
    "src/ts3_control.cpp"
    "src/wvw.cpp"
)
source_group("Source Files" FILES ${Taco_Source_Files})

set(Source_Files
    "src/gw2_pois.cpp"
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
    "Data/trail.png"
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
    ${Taco_Source_Files}
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
# Libraries
################################################################################
add_library(taco_util STATIC)
target_sources(taco_util
    PRIVATE
        ${Util_Source_Files}
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES
        FILES
            src/util/xml_node.ixx
            src/util/xml_document.ixx
            src/util/xml.ixx
)
target_include_directories(taco_util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(taco_util PRIVATE third_party)
use_props(taco_util "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

add_library(taco_core2 STATIC)
target_sources(taco_core2
    PRIVATE
        ${Core2_Source_Files}
)
target_include_directories(taco_core2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(taco_core2 PRIVATE taco_base taco_util)
use_props(taco_core2 "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

add_library(whiteboard STATIC)
target_sources(whiteboard
    PRIVATE
        ${Whiteboard_Source_Files}
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES
        FILES
            src/white_board/atlas.ixx
            src/white_board/application.ixx
            src/white_board/box.ixx
            src/white_board/button.ixx
            src/white_board/context_menu.ixx
            src/white_board/css_item.ixx
            src/white_board/draw_api.ixx
            src/white_board/font.ixx
            src/white_board/gui_item.ixx
            src/white_board/label.ixx
            src/white_board/message_enum.ixx
            src/white_board/message.ixx
            src/white_board/root.ixx
            src/white_board/skin.ixx
            src/white_board/style_manager.ixx
            src/white_board/text_box.ixx
            src/white_board/white_board.ixx
            src/white_board/window.ixx
)
target_include_directories(whiteboard PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(whiteboard PRIVATE taco_base taco_core2 third_party)
use_props(whiteboard "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

################################################################################
# Taco Library
################################################################################
add_library(taco STATIC)
target_sources(taco
    PRIVATE
        ${Taco_Source_Files}
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES
        FILES
            src/build_info.ixx
            src/dungeon_progress.ixx
            src/gw2_api.ixx
            src/gw2_taco.ixx
            src/hp_grid.ixx
            src/language.ixx
            src/locational_timer.ixx
            src/map_timer.ixx
            src/marker_data.ixx
            src/marker_editor.ixx
            src/mouse_highlight.ixx
            src/mumble_link.ixx
            src/notepad.ixx
            src/overlay_application.ixx
            src/overlay_config.ixx
            src/overlay_window.ixx
            src/poi_behavior.ixx
            src/pro_font.ixx
            src/raid_progress.ixx
            src/range_display.ixx
            src/special_gui_items.ixx
            src/string_set.ixx
            src/tactical_category.ixx
            src/tactical_compass.ixx
            src/tp_tracker.ixx
            src/trail_logger.ixx
            src/ts3_connection.ixx
            src/ts3_control.ixx
            src/wvw.ixx
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/.."
        FILES ${Header_Files}
)
target_include_directories(taco PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(taco PUBLIC
    taco_base
    taco_util
    taco_core2
    whiteboard
    third_party
)
use_props(taco "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        ${Source_Files}
)

add_subdirectory(src/base)

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
# Compile definitions
################################################################################
target_compile_definitions(taco_util PUBLIC ${Common_Definitions})
target_compile_definitions(taco_core2 PUBLIC ${Common_Definitions})
target_compile_definitions(whiteboard PUBLIC ${Common_Definitions})
target_compile_definitions(taco PUBLIC
    ${Common_Definitions}
    "MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${Common_Definitions}
    "MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
        target_compile_options(taco PRIVATE
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
            /Y-
        )
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
    taco
)


install(
    TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "."
)
install(
    FILES ${Resources} DESTINATION "."
)
install(
    FILES ${Data_Files} DESTINATION "Data"
)
install(
    FILES ${Poi_Files} DESTINATION "POIs"
)
install(
    FILES ${Poi_Data_Files} DESTINATION "POIs/Data"
)

################################################################################
# CPACK
################################################################################

set(CPACK_GENERATOR "ZIP")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${TACO_VERSION})
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
include(CPack)
