# Software 3D Renderer

A small software rasterizer / renderer written in C using SDL2 and SDL2_ttf for windowing and text rendering.

## Features

- Tile-aware software rasterization
- OBJ model loading (via `obj_loader.c`)
- Camera controls (WASD + mouse)
- Switch models with keys `1`, `2`, `3`, `4`
- On-screen FPS and debug info via SDL_ttf

## Setup & Build (Detailed Windows instructions)

1. Install Visual Studio (or Build Tools)

- Install Visual Studio 2019/2022 or the "Build Tools for Visual Studio" and make sure the **Desktop development with C++** workload is installed. Ensure you can run a Developer Command Prompt and that `cl.exe` is available.

2. Download SDL2 and SDL2_ttf (development packages)

- SDL2: https://www.libsdl.org/download-2.0.php — download the "SDL2-devel-<version>-VC" zip.
- SDL2_ttf: https://www.libsdl.org/projects/SDL_ttf/ — download the VC development zip.

3. Place headers/libs into this repo layout (example)

- Copy the SDL2 headers into `include\\SDL2\\` (so `include\\SDL2\\SDL.h` exists).
- Copy SDL2_ttf headers into `include\\SDL2\\SDL2_ttf-<version>\\include` or into `include\\SDL2` if you prefer a flat layout.
- Copy the x64 libraries and DLLs into `lib\\x64\\` (for example `lib\\x64\\SDL2.lib`, `lib\\x64\\SDL2.dll`, `lib\\x64\\SDL2_ttf.lib`, `lib\\x64\\SDL2_ttf.dll`).

This repository already includes a `lib\\x64` folder; confirm the DLLs (_.dll) and import libs (_.lib) are present and match the architecture (x64).

4. Build using the provided script

- Open "x64 Native Tools Command Prompt for VS" (or run the Developer Command Prompt and call `vcvarsall.bat x64`).
- From the repository root run:

```powershell
.\\build.bat
```

What the script does:

- Initializes the x64 MSVC environment (attempts to call `vcvarsall.bat x64`).
- Compiles `src\\*.c` with `/I include` and `/I include\\SDL2`.
- Links against libraries in `lib\\x64` including `SDL2.lib`, `SDL2main.lib`, and `SDL2_ttf.lib`.
- Copies `lib\\x64\\SDL2.dll` and `lib\\x64\\SDL2_ttf.dll` into `bin\\` for runtime.

5. Common problems and fixes

- Link errors (e.g. unresolved `TTF_Init`): make sure you're linking `SDL2_ttf.lib` and that the library's architecture matches your compiler (x64 vs x86).
- Missing DLL at runtime: ensure `SDL2.dll` and `SDL2_ttf.dll` are present in `bin\\` or on `PATH`.
- If `build.bat` fails to find Visual Studio scripts, open the Developer Command Prompt manually, which preconfigures environment variables.

Alternative install options

- vcpkg: If you use `vcpkg`, you can `vcpkg install sdl2 sdl2-ttf` and integrate with MSBuild. That will keep system dependencies out of the repo.
- MSYS2: `pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf` if targeting MinGW.

6. Adjusting asset paths

- The code uses relative asset paths like `..\\assets\\teapot.obj` — running the executable from `bin\\` may require changing paths or copying assets into `bin\\assets`. If you prefer, I can update `build.bat` to copy `assets` into `bin` after build.

If you'd like, I can also add step-by-step commands to use `vcpkg` or update `build.bat` to automatically copy `assets` into `bin`.

## Controls

- WASD: move camera
- Mouse: look around (relative mode enabled)
- Space: move up
- Left Shift: move down
- C: clear the software buffer
- 1 / 2 / 3 / 4: load different OBJ meshes (dog, bunny, teapot, dragon)

## Notes & Troubleshooting

- If you see linker errors like `unresolved external symbol TTF_Init`, ensure you are linking `SDL2_ttf.lib` from the same architecture (x64) as your compiler.
- If text or TTF functions fail at runtime, ensure `SDL2_ttf.dll` is present in `bin\` alongside `main.exe`.
- Asset paths in code are relative (e.g. `..\assets\teapot.obj`). Run the executable from the repository root or adjust paths if running from `bin`.

## Project Structure

- `src/` — source files
- `include/` — project headers and bundled SDL headers
- `lib/x64/` — third-party `.lib`/`.dll` files
- `assets/` — models, fonts, and other data

