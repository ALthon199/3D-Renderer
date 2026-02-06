#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "geometry.h"

// The Window: Handles screen and SDL management

#define WIDTH 1200
#define HEIGHT 800

typedef struct Display {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* pixels;
    float* z_buffer;
} Display;


typedef struct {
    float mouseX;
    float mouseY;
    bool pressed;
} Mouse;


Display* init_window(void);
void clear_screen(Display* display);
void present_frame(Display* display);
void process_events(Display* display, bool* running, Mouse* mouse);
void cleanup_window(Display* display);
uint32_t* get_pixels(Display* display);

#endif

