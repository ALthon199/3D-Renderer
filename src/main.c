#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "display.h"
#include "renderer.h"
#include "obj_loader.h"

int main(int argc, char* argv[]) {
    // Initialize display
    Display* display = init_window();
    
    // State
    Mouse mouse = {0};
    Camera camera = {{0, 0, 0}, 0, 0};
    Keyboard keyboard;
    keyboard.keyboard_state = SDL_GetKeyboardState(NULL);
    
    Mesh teapot;


    int opened = load_obj("..\\assets\\xyzrgb_dragon.obj", &teapot);
    float old_mouseX = 0;
    float old_mouseY = 0;
    bool running = true;
    

    Mesh cube_mesh;
    // 8 Vertices for a unit cube
    // AI created colors for each vertex for testing
    Vertex cube_vertices[] = {
    {.position = {-1,-1, -1}, .color = 0xFFFF0000}, // 0: Bottom-Left-Back (Red)
    {.position = { 1, -1, -1}, .color = 0xFFFF0000}, // 1: Bottom-Right-Back (Green)
    {.position = { 1,  1, -1}, .color = 0xFFFFFF00}, // 2: Top-Right-Back (Blue)
    {.position = {-1,  1, -1}, .color = 0xFFFFFF00}, // 3: Top-Left-Back (White)
    {.position = {-1, -1,  1}, .color = 0xFFFF0000}, // 4: Bottom-Left-Front (Yellow)
    {.position = { 1, -1,  1}, .color = 0xFFFF0000}, // 5: Bottom-Right-Front (Magenta)
    {.position = { 1,  1,  1}, .color = 0xFFFFFF00}, // 6: Top-Right-Front (Cyan)
    {.position = {-1,  1,  1}, .color = 0xFFFFFF00}  // 7: Top-Left-Front (Black)
    };

    double start_time = SDL_GetTicks();
    
    while (running) {
        
        old_mouseX = mouse.mouseX;
        old_mouseY = mouse.mouseY;
        process_events(display, &running, &mouse, &camera, &keyboard);
        
 
    
        draw_mesh(display, &teapot, &camera); 
        
      
        double current_time = SDL_GetTicks();
        double elapsed_time = current_time - start_time;
        printf("FPS: %.2f\n", 1000.0 / elapsed_time);
        start_time = current_time;
     
        present_frame(display);
        
        SDL_Delay(10);
        clear_screen_with_gradient(display);
        
        
    }
    
    // Cleanup
    cleanup_window(display);
    free_mesh(&teapot);
    return 0;
}