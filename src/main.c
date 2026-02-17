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
    int cube_indices[] = {
    // Front face (Z+)
    4, 6, 5,  4, 7, 6,
    // Back face (Z-)
    0, 1, 2,  0, 2, 3,
    // Left face (X-)
    0, 3, 7,  0, 7, 4,
    // Right face (X+)
    1, 5, 6,  1, 6, 2,
    // Top face (Y+)
    3, 2, 6,  3, 6, 7,
    // Bottom face (Y-)
    0, 4, 5,  0, 5, 1
    };  
    
    cube_mesh.Vertices = cube_vertices;
    cube_mesh.vertex_count = sizeof(cube_vertices)/sizeof(Vertex);
    cube_mesh.indices = cube_indices;
    cube_mesh.index_count = sizeof(cube_indices) / sizeof(int);
    cube_mesh.rotation_angle = 0.0f;
    cube_mesh.translation = 5.0f;
    while (running) {
        
        old_mouseX = mouse.mouseX;
        old_mouseY = mouse.mouseY;
        process_events(display, &running, &mouse, &camera, &keyboard);
        
 
       
        teapot.rotation_angle += 0.1f;
        
        draw_mesh(display, &teapot, &camera); 
        
      
     
     
        present_frame(display);
        
        SDL_Delay(10);
        clear_screen_with_gradient(display);

        
    }
    
    // Cleanup
    cleanup_window(display);
    free_mesh(&teapot);
    return 0;
}