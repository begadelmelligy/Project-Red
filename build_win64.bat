@echo off
setlocal

set SRC=src\main.c src\game.c
set OUTPUT=bin\game.exe

set RAYLIB_INCLUDE=deps\RAYLIB\include
set RAYLIB_LIB=deps\RAYLIB\lib

set RAYGUI_INCLUDE=deps\RAYGUI

if not exist bin mkdir bin

set CFLAGS=-Wall -g
set LIBS=-lraylib -lopengl32 -lgdi32 -lwinmm

echo.
echo Building...
echo -----------------------------------------
cc %CFLAGS% ^
    %SRC% -o %OUTPUT% ^
    -I"%RAYLIB_INCLUDE%" ^
    -I"%RAYGUI_INCLUDE%" ^
    -L"%RAYLIB_LIB%" ^
    %LIBS%

if %ERRORLEVEL% neq 0 (
    echo -----------------------------------------
    echo Build failed! :(
    echo.
    exit /b %ERRORLEVEL%
)

echo Build successful! Launching the game...
call %OUTPUT%

echo -----------------------------------------
echo. 
echo EXIT successfully! ^^_^^
echo.                 
exit /b 0
