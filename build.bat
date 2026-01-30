@echo off
cd /d "%~dp0"

:: 1. Check for cl.exe (Visual Studio Compiler)
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Searching for Visual Studio Compiler...
    
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat"
)

echo Building from Root...


if not exist bin mkdir bin

:: 2. Compile the source files
:: 2. Compile the source files (Updated to include all .c files)
cl /I "include\SDL2" /I "include" src\*.c /Fe"bin\main.exe" /Fo"bin\\" /link /LIBPATH:"lib\x86" SDL2.lib SDL2main.lib shell32.lib user32.lib gdi32.lib /SUBSYSTEM:CONSOLE
if %errorlevel% neq 0 (
    echo BUILD FAILED!
    pause
    exit /b %errorlevel%
)

:: 4. Copy DLL to bin
if exist SDL2.dll copy /Y SDL2.dll bin\ >nul

echo SUCCESS! Attempting to run...
pushd bin
if exist main.exe (
    main.exe
) else (
    echo FATAL: main.exe not found in bin folder!
    dir
)
popd
pause