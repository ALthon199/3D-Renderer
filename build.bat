@echo off
cd /d "%~dp0"

:: 1. Check for cl.exe (Visual Studio Compiler)
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Searching for Visual Studio Compiler...
    
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    ) else if exist "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    ) else if exist "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        echo WARNING: Could not find Visual Studio vcvars script. Make sure VS is installed or run the "Developer Command Prompt for VS" first.
    )
)

echo Building from Root...


if not exist bin mkdir bin

:: 2. Compile the source files
:: 2. Compile the source files (Updated to include all .c files)
cl /O2 /openmp /I "include\SDL2" /I "include" /I "include\SDL2\SDL2_ttf-2.24.0\include" src\*.c /Fe"bin\main.exe" /Fo"bin\\" /link /LIBPATH:"lib\x64" SDL2.lib SDL2main.lib SDL2_ttf.lib shell32.lib user32.lib gdi32.lib legacy_stdio_definitions.lib /SUBSYSTEM:CONSOLE
if %errorlevel% neq 0 (
    echo BUILD FAILED!
    pause
    exit /b %errorlevel%
)

:: 4. Copy DLL to bin
if exist "lib\x64\SDL2.dll" copy /Y "lib\x64\SDL2.dll" bin\ >nul
if exist "lib\x64\SDL2_ttf.dll" copy /Y "lib\x64\SDL2_ttf.dll" bin\ >nul

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