@echo off

md ..\..\build
pushd ..\..\build
cl /FC /Zi ..\code\win32_platform.cpp user32.lib gdi32.lib
popd
