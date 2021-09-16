@echo off

md ..\..\build
pushd ..\..\build
cl /Zi ..\code\win32_platform.cpp user32.lib gdi32.lib
popd
