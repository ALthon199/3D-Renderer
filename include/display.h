#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "geometry.h"

// The Window: Handles screen and SDL management

#define WIDTH 1200
#define HEIGHT 800
#define MAX_TRIANGLES 2000000 // For tile-based rasterization optimization
#define TILE_SIZE 16 // For tile-based rasterization optimization
typedef struct Display {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Tile* tiles; // For tile-based rasterization optimization
    int* global_tiles; // For tile-based rasterization optimization
    int* fill_tracker; // For tile-based rasterization optimization
    int max_triangles; // For tile-based rasterization optimization
    uint32_t* pixels;
    float* z_buffer;
} Display;

typedef struct{
    const uint8_t* keyboard_state; // SDL supports up to 512 scancodes
} Keyboard;

typedef struct {
    float mouseX;
    float mouseY;
    bool pressed;
} Mouse;

typedef struct {
    Vector3 position;
    float pitch;
    float yaw;
} Camera;

Display* init_window(void);
void clear_screen(Display* display);
void clear_screen_with_gradient(Display* display);
void present_frame(Display* display);
void process_events(Display* display, bool* running, Mouse* mouse, Camera* camera, Keyboard* keyboard);
void cleanup_window(Display* display);
uint32_t* get_pixels(Display* display);

// Create a grid floor mesh (allocates vertices/indices).
void create_grid_floor(Mesh* mesh, int segments, float size, float y_level);

#endif

