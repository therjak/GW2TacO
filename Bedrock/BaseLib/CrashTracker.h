#pragma once

#include <Windows.h>

#include <string_view>

LONG WINAPI baseCrashTracker(struct _EXCEPTION_POINTERS* excpInfo);
LONG WINAPI FullDumpCrashTracker(struct _EXCEPTION_POINTERS* excpInfo);
void InitializeCrashTracker(
    std::string_view Build,
    LPTOP_LEVEL_EXCEPTION_FILTER Tracker = baseCrashTracker);
