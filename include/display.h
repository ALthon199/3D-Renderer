#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <SDL_ttf.h>
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
    TTF_Font* text_renderer;
    SDL_Texture* texture;
    SDL_Texture* rendered_text[10]; // For rendering text (FPS, debug info, etc.)

    Tile* tiles; // For tile-based rasterization optimization
    int* global_tiles; // For tile-based rasterization optimization
    int* fill_tracker; // For tile-based rasterization optimization
    int max_triangles; // For tile-based rasterization optimization

    // Pixel data for the current frame (ARGB format)
    uint32_t* pixels;
    // Z-buffer for depth
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
void process_events(Display* display, bool* running, Mouse* mouse, Camera* camera, Keyboard* keyboard, Mesh* mesh, char* debug_info);
void cleanup_window(Display* display);
uint32_t* get_pixels(Display* display);

// Create a grid floor mesh (allocates vertices/indices).
void create_grid_floor(Mesh* mesh, int segments, float size, float y_level);


void update_fps_text(Display* display, int frames_per_second);
void key_info_text(Display* display, const char* key_info);
void update_debug_text(Display* display, const char* debug_info);
void update_render_info_text(Display* display, const char* render_info);
void update_face_count_text(Display* display, int face_count);
void update_text(Display* display, int* current_time, int* last_time, int* frames_per_second, int face_count, const char* debug_info, const char* render_info);
#endif

