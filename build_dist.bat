@echo off
echo Building MarkIt Distribution...

:: Create a fresh release build directory
if exist build_release rmdir /s /q build_release

cmake -B build_release -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

cmake --build build_release --config Release
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo Distribution packages created in build_release directory!
