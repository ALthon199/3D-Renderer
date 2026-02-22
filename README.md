# Software 3D Renderer

A small software rasterizer / renderer written in C using SDL2 and SDL2_ttf for windowing and text rendering.

## Features

- Tile-aware software rasterization
- OBJ model loading (via `obj_loader.c`)
- Camera controls (WASD + mouse)
- Switch models with keys `1`, `2`, `3`, `4`
- On-screen FPS and debug info via SDL_ttf

## Prerequisites (Windows)

- Microsoft Visual Studio (cl.exe) with x64 toolchain
- SDL2 development files (headers + x64 libs)
- SDL2_ttf development files (headers + x64 libs)

The repository contains a local `lib\x64` folder with `SDL2.lib`, `SDL2.dll`, `SDL2_ttf.lib`, and `SDL2_ttf.dll` used by the build script.

## Build (recommended)

Open a "Developer Command Prompt for VS" configured for x64 or run the included `build.bat` which tries to call `vcvarsall.bat x64`.

From the repository root:

```powershell
.\build.bat
```

What `build.bat` does:

- Calls Visual Studio vcvars (x64) if available
- Compiles `src\*.c` with include paths `include` and `include\SDL2`
- Links `lib\x64\SDL2.lib`, `SDL2main.lib`, and `SDL2_ttf.lib`
- Copies `lib\x64\SDL2.dll` and `lib\x64\SDL2_ttf.dll` into `bin\`

If you prefer to compile manually with `cl`, add the following include paths:

```
/I include /I include\SDL2 /I include\SDL2\SDL2_ttf-2.24.0\include
```

and link against libraries in `lib\x64`.

## Run

After a successful build run `bin\main.exe` (the build script attempts to run it automatically).

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

If you'd like, I can also add a small step to copy assets into `bin` automatically or adjust asset paths to be relative to the executable.
Dependencies:
This project requires SDL2 (x86).

Download the SDL2 VC development transition from libsdl.org.

Place the include\SDL2 and lib\(x86 or x64) folders in the project root.

Run build.bat.
