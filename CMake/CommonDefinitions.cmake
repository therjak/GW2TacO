set(Common_Definitions
    "$<$<CONFIG:Debug_MultiByte>:_DEBUG>"
    "$<$<CONFIG:Debug>:_DEBUG>"
    "$<$<CONFIG:Release_MultiByte>:NDEBUG>"
    "$<$<CONFIG:Release>:NDEBUG>"
    "NOMINMAX"
    "_CRT_SECURE_NO_WARNINGS"
    "WIN32_LEAN_AND_MEAN"
    "WIN32"
    "_WINDOWS"
    CACHE INTERNAL "Project wide definitions"
)
