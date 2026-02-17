#include <SDL.h>
#include <string.h>
#include <stdlib.h>
#include "display.h"
#include "renderer.h"
#include "vector_math.h"

Display* init_window(void) {
    Display* display = (Display*)malloc(sizeof(Display));
    
    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetRelativeMouseMode(SDL_TRUE);
   
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
        display->z_buffer[i] = 10000.0f; // Reset z-buffer
    }
}
void clear_screen_with_gradient(Display* display) {
    for (int y = 0; y < HEIGHT; y++) {
        // Calculate a value from 0.0 (top) to 1.0 (bottom)
        float t = (float)y / (float)HEIGHT;
        
        // Interpolate between a dark top and a slightly lighter bottom
        uint8_t r = (uint8_t)(30 * (1.0f - t) + 60 * t);
        uint8_t g = (uint8_t)(30 * (1.0f - t) + 60 * t);
        uint8_t b = (uint8_t)(35 * (1.0f - t) + 65 * t);

        uint32_t row_color = (0xFF << 24) | (r << 16) | (g << 8) | b;
        
        for (int x = 0; x < WIDTH; x++) {
            display->pixels[y * WIDTH + x] = row_color;
            display->z_buffer[y * WIDTH + x] = 10000; // Reset Z-buffer
        }
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

void process_events(Display* display, bool* running, Mouse* mouse, Camera* camera, Keyboard* keyboard) {
    SDL_Event e;
    
    
    while (SDL_PollEvent(&e)) {

        if (e.type == SDL_QUIT) {
            *running = false;
        }

        
        if (e.type == SDL_MOUSEMOTION) {
            
            mouse->mouseX = (float)e.motion.x;
            mouse->mouseY = (float)e.motion.y;
            camera -> yaw += 0.001f*(float)e.motion.xrel;
            camera -> pitch -= 0.001f*(float)e.motion.yrel;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            mouse->pressed = true;
        }

        if (e.type == SDL_MOUSEBUTTONUP) {
            mouse->pressed = false;
        }
        

        
    }
        
    // Clear screen if 'C' is pressed
    if (keyboard->keyboard_state[6]) { // 'C' key scancode is 6
        clear_screen(display);
    }
    Vector3 forward_vec = forward(camera);
    Vector3 right_vec = vec_cross(forward_vec, (Vector3){0,1.0f,0});
    
    
    if (keyboard->keyboard_state[26]) { // 'W' key
        camera->position.x += forward_vec.x * 0.04f;
        
        camera->position.z += forward_vec.z * 0.04f;
    }
    if (keyboard->keyboard_state[22]) { // 'S' key
        camera->position.x -= forward_vec.x * 0.04f;
        
        camera->position.z -= forward_vec.z * 0.04f;
    }
    
    if (keyboard->keyboard_state[4]) { // 'A' key
        camera->position.x += right_vec.x * 0.04f;
        camera->position.y += right_vec.y * 0.04f;
        camera->position.z += right_vec.z * 0.04f;
    }
    
    if (keyboard->keyboard_state[7]) { // 'D' key
    
        camera->position.x -= right_vec.x * 0.04f;
        camera->position.y -= right_vec.y * 0.04f;
        camera->position.z -= right_vec.z * 0.04f;
    }
    
    if (keyboard->keyboard_state[44]) { // Space key
        camera->position.y += 0.04f;
    }

    if (keyboard->keyboard_state[225]) { // Left Shift key
        camera->position.y -= 0.04f;
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


