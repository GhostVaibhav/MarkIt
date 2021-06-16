@echo off


echo Cloning repo from Github
git clone https://github.com/curl/curl.git
git clone -b master --single-branch https://github.com/nlohmann/json.git
git clone https://github.com/wmcbrine/PDCurses.git
cd curl > nul
cd winbuild > nul



echo Calling Visual Studio
call "%vs90comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs100comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs110comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs120comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs140comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs140comntools%\VsDevCmd.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86 > nul 2> nul



echo Building curl using Visual Studio
nmake /f Makefile.vc DEBUG=no mode=dll > nul



echo Cleaning the repo
cd .. > nul
cd builds > nul
cd libcurl-vc-x86-release-dll-ipv6-sspi-schannel > nul
cd bin > nul
del curl.exe > nul 2> nul
cd .. > nul
cd lib > nul
del libcurl.exp > nul 2> nul
cd .. > nul



echo Copying the files in the app
xcopy /E /I /Y "bin" "../../../bin" > nul 2> nul
xcopy /E /I /Y "lib" "../../../lib" > nul 2> nul
xcopy /E /I /Y "include" "../../../include" > nul 2> nul
cd .. > nul
cd .. > nul
cd .. > nul



echo Removing curl repo
RMDIR /Q/S curl


echo Copying JSON file
cd json > nul
cd single_include > nul
cd nlohmann > nul
xcopy /E /I /Y "json.hpp" "../../../include" > nul 2> nul


echo Removing JSON repo
cd .. > nul
cd .. > nul
cd .. > nul
RMDIR /Q/S json



echo Building PDCurses library
cd PDCurses
copy /Y "curses.h" "../include" > nul 2> nul
cd wincon


echo Calling Visual Studio
call "%vs90comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs100comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs110comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs120comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs140comntools%vsvars32.bat" x86 > nul 2> nul
call "%vs140comntools%\VsDevCmd.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86 > nul 2> nul
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x86 > nul 2> nul


echo.
echo.
echo Build using which system?
echo 1. MinGW
echo 2. Visual Studio
SET cmd=%~1
IF %cmd%==2 (
    echo Building using Visual Studio
    nmake /f Makefile.vc WIDE=Y > nul
) 
IF %cmd%==1 (
    echo Building using Mingw
    make -f Makefile WIDE=Y INFOEX=N > nul
) 
IF %cmd%=="" (
    set /p choice= "Please select one of the above options (1/2):"
    IF "%choice%"==2 (
        echo Building using Visual Studio
        nmake /f Makefile.vc WIDE=Y > nul
    ) else (
        echo Building using Mingw
        make -f Makefile WIDE=Y INFOEX=N > nul
    )
)

echo Renaming files
ren "pdcurses.a" "libpdcurses.a" > nul 2> nul
ren "pdcurses.lib" "libpdcurses.lib" > nul 2> nul

echo Moving files
xcopy /E /I /Y "libpdcurses.a" "../../lib" > nul 2> nul
xcopy /E /I /Y "libpdcurses.lib" "../../lib" > nul 2> nul
cd ..


echo Removing repo PDCurses
cd ..
RMDIR /Q/S PDCurses