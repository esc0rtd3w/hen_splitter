@echo off
set VSDIR="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC"
call %VSDIR%\vcvarsall.bat x86
nmake -f Makefile
pause