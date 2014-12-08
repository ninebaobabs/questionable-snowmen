@echo off

mkdir ..\build

pushd ..\build
cl -O2 ..\code\win32_game.cpp user32.lib gdi32.lib
popd