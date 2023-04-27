@echo off
cls
echo Building Hen Splitter EXE...
echo.
set VSDIR="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC"
call %VSDIR%\vcvarsall.bat x86
nmake -f Makefile
echo.
echo done.
echo.
pause