@echo off
start /b "" "%~dp0mumble_link_bridge.exe" -quiet
"%~dp0Gw2-64.exe" %*
taskkill /f /im mumble_link_bridge.exe >nul 2>&1
