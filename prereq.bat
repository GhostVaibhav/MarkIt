@echo off

SET generator=%~1%
SET generatorString=

IF NOT "%generator%" == "" (
    SET generatorString=-G "%generator%"
)

call :colorEcho 0a "[PREREQ] - Preparing to install [Curl]"
cd "deps/curl"

@RD /S /Q ".\library"
@RD /S /Q ".\build"
mkdir library
mkdir build && cd build
echo.

call :colorEcho 0a "[PREREQ] - Configuring Curl"
echo.
cmake .. %generatorString% -DCMAKE_BUILD_TYPE=%buildType% -DCMAKE_INSTALL_PREFIX=../library -DBUILD_CURL_EXE=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DCURL_USE_SCHANNEL=ON -DUSE_WIN32_LDAP=ON -DENABLE_THREADED_RESOLVER=OFF -DUSE_LIBIDN2=OFF
echo.
call :colorEcho 0a "[PREREQ] - Building Curl"
echo.
cmake --build . --config Release
echo.
call :colorEcho 0a "[PREREQ] - Installing Curl"
echo.
cmake --install . --config Release

cd "../../.."
echo.

call :colorEcho 0a "[PREREQ] - Preparing to install [PDCurses]"
cd "deps/PDCurses"

@RD /S /Q ".\library"
@RD /S /Q ".\build"
mkdir library
mkdir build && cd build
echo.

call :colorEcho 0a "[PREREQ] - Configuring PDCurses"
echo.
cmake .. %generatorString% -DCMAKE_BUILD_TYPE=%buildType% -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=../library
echo.
call :colorEcho 0a "[PREREQ] - Building PDCurses"
echo.
cmake --build . --config Release
echo.
call :colorEcho 0a "[PREREQ] - Installing PDCurses"
echo.
cmake --install . --config Release
echo.
call :colorEcho 0a "[PREREQ] - Installed [Curl, PDCurses]"
echo.

exit
:colorEcho
echo off
<nul set /p ".=%DEL%" > "%~2"
findstr /v /a:%1 /R "^$" "%~2" nul
del "%~2" > nul 2>&1i
