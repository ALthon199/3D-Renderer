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


    int opened = load_obj("..\\assets\\dog.obj", &teapot);
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


    Mesh floor_mesh;
    create_grid_floor(&floor_mesh, 10, 100.0f, -1.5f);
    



    double start_time = SDL_GetTicks();
    
    while (running) {
        clear_screen_with_gradient(display);
        old_mouseX = mouse.mouseX;
        old_mouseY = mouse.mouseY;
        process_events(display, &running, &mouse, &camera, &keyboard);

    
    
        draw_mesh(display, &teapot, &camera); 
        teapot.rotation_angle += 0.02f;


        transform_mesh(display, &floor_mesh, &camera); // Pre-transform floor mesh vertices to compute gradients for it
        for (int i = 0; i < floor_mesh.index_count; i+=3) {
            int ia = floor_mesh.indices[i];
            int ib = floor_mesh.indices[i+1];
            int ic = floor_mesh.indices[i+2];
            // Near-plane guard: skip triangles with any vertex too close or behind camera
            float za = floor_mesh.camera_vertices[ia].z;
            float zb = floor_mesh.camera_vertices[ib].z;
            float zc = floor_mesh.camera_vertices[ic].z;
            // Skip triangle only if all three vertices are behind/too close to near plane
            if (za < 0.1f && zb < 0.1f && zc < 0.1f) continue;

            Vertex v0 = floor_mesh.projected_vertices[ia];
            Vertex v1 = floor_mesh.projected_vertices[ib];
            Vertex v2 = floor_mesh.projected_vertices[ic];
            Triangle tri = {v0, v1, v2, {0}};

            tri.v0.color = floor_mesh.shaded_colors[ia];
            tri.v1.color = floor_mesh.shaded_colors[ib];
            tri.v2.color = floor_mesh.shaded_colors[ic];
            draw_triangle(display, &tri);
        }
        
        double current_time = SDL_GetTicks();
        double elapsed_time = current_time - start_time;
        
        start_time = current_time;
     
        present_frame(display);
        
        SDL_Delay(10);
        
        
        
    }
    
    // Cleanup
    cleanup_window(display);
    free_mesh(&teapot);
    free_mesh(&floor_mesh);
    return 0;
}