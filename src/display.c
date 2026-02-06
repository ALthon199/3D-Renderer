#include <SDL.h>
#include <string.h>
#include <stdlib.h>
#include "display.h"
#include "renderer.h"

Display* init_window(void) {
    Display* display = (Display*)malloc(sizeof(Display));
    
    SDL_Init(SDL_INIT_VIDEO);
    
    display->window = SDL_CreateWindow("3D Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
    display->texture = SDL_CreateTexture(display->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    
    display->pixels = (uint32_t*)malloc(sizeof(uint32_t) * WIDTH * HEIGHT);
    memset(display->pixels, 0, sizeof(uint32_t) * WIDTH * HEIGHT);
    display->z_buffer = (float*)malloc(sizeof(float) * WIDTH * HEIGHT);
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        display->z_buffer[i] = 1000.0f; // Initialize z-buffer to farthest depth
    }
    return display;
}

void clear_screen(Display* display) {
    memset(display->pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        display->z_buffer[i] = 1000.0f; // Reset z-buffer
    }
}

void present_frame(Display* display) {
    SDL_UpdateTexture(display->texture, NULL, display->pixels, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(display->renderer);
    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}

uint32_t* get_pixels(Display* display) {
    return display->pixels;
}

void process_events(Display* display, bool* running, Mouse* mouse) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *running = false;
        }

        if (e.type == SDL_MOUSEMOTION) {
            
            mouse->mouseX = (float)e.motion.x;
            mouse->mouseY = (float)e.motion.y;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            mouse->pressed = true;
        }

        if (e.type == SDL_MOUSEBUTTONUP) {
            mouse->pressed = false;
        }
        
        // Clear screen if 'C' is pressed
        if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == 6) {
            clear_screen(display);
        }
        
        // Draw random triangles if 'D' is pressed
        if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == 7) {
            for (int i = 0; i < 1000; i++) {
                Vertex v0 = {rand() % WIDTH, rand() % HEIGHT, (float) rand() / RAND_MAX, 0xFF0000FF};
                Vertex v1 = {rand() % WIDTH, rand() % HEIGHT, (float) rand() / RAND_MAX, 0xFF00FF00};
                Vertex v2 = {rand() % WIDTH, rand() % HEIGHT, (float) rand() / RAND_MAX, 0xFF00FFFF};
                Triangle tri = {v0, v1, v2};
                draw_triangle(display, tri);
            }
        }
        
        // Draw single triangle if 'E' is pressed
        if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == 8) {
            // 2. Define the FAR triangle
            Vertex v0 = { 100, 100, 0.1f, 0xFFFF0000 };
            Vertex v1 = { 400, 100, 0.9f, 0xFFFF0000 };
            Vertex v2 = { 250, 400, 0.9f, 0xFFFF0000 };
            Triangle far_tri = { v0, v1, v2 };

            // 3. Define the NEAR triangle (offset slightly so they overlap)
            Vertex v3 = { 150, 150, 0.5f, 0xFF0000FF };
            Vertex v4 = { 450, 150, 0.5f, 0xFF0000FF };
            Vertex v5 = { 300, 450, 0.5f, 0xFF0000FF };
            Triangle near_tri = { v3, v4, v5 };
            draw_triangle(display, near_tri);
            draw_triangle(display, far_tri);
        }
    }
}

void cleanup_window(Display* display) {
    free(display->pixels);
    free(display->z_buffer);
    SDL_DestroyTexture(display->texture);
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    SDL_Quit();
    free(display);
}


