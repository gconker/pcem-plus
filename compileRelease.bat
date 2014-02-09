@echo off
set path=%PATH%;mingw\bin
mingw32-make.exe -f Makefile.mingw release
pause