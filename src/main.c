#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "display.h"
#include "renderer.h"

int main(int argc, char* argv[]) {
    // Initialize display
    Display* display = init_window();
    
    // State
    Mouse mouse = {0};
    float old_mouseX = 0;
    float old_mouseY = 0;
    bool running = true;

    Mesh cube_mesh;
    // 8 Vertices for a unit cube
    // AI created colors for each vertex for testing
    Vertex cube_vertices[] = {
    {-1, -1, -1, 0xFFFF0000}, // 0: Bottom-Left-Back (Red)
    { 1, -1, -1,  0xFFFF0000}, // 1: Bottom-Right-Back (Green)
    { 1,  1, -1,  0xFFFFFF00}, // 2: Top-Right-Back (Blue)
    {-1,  1, -1,  0xFFFFFF00}, // 3: Top-Left-Back (White)
    {-1, -1,  1,  0xFFFF0000}, // 4: Bottom-Left-Front (Yellow)
    { 1, -1,  1, 0xFFFF0000}, // 5: Bottom-Right-Front (Magenta)
    { 1,  1,  1, 0xFFFFFF00}, // 6: Top-Right-Front (Cyan)
    {-1,  1,  1, 0xFFFFFF00}  // 7: Top-Left-Front (Black)
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
    cube_mesh.indices = cube_indices;
    cube_mesh.index_count = sizeof(cube_indices) / sizeof(int);
    cube_mesh.rotation_angle = 0.0f;
    cube_mesh.translation = 5.0f;
    while (running) {
        
        old_mouseX = mouse.mouseX;
        old_mouseY = mouse.mouseY;
        process_events(display, &running, &mouse);
        
       
       
        cube_mesh.rotation_angle += 0.02f;
        
        draw_mesh(display, &cube_mesh); 
            
        
        
        
     
        present_frame(display);
        
        SDL_Delay(10);
        clear_screen(display);
    }
    
    // Cleanup
    cleanup_window(display);
    
    return 0;
}