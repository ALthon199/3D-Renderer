#include <SDL.h>
#include <SDL_ttf.h>
#include <string.h>
#include <stdlib.h>
#include "display.h"
#include "renderer.h"
#include "vector_math.h"

Display* init_window(void) {
    Display* display = (Display*)malloc(sizeof(Display));
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        free(display);
        return NULL;
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        free(display);
        return NULL;
    }


    SDL_SetRelativeMouseMode(SDL_TRUE);
   
    display->window = SDL_CreateWindow("3D Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
    display->text_renderer = TTF_OpenFont("..\\assets\\font\\JetBrainsMono-Regular.ttf", 24);
    display->rendered_text[0] = NULL; // Initialize rendered text slots to NULL

    display->texture = SDL_CreateTexture(display->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    display->pixels = (uint32_t*)malloc(sizeof(uint32_t) * WIDTH * HEIGHT);
    display->tiles = (Tile*)malloc(sizeof(Tile) * ((WIDTH / TILE_SIZE) * (HEIGHT / TILE_SIZE))); // For tile-based rasterization optimization
    display->global_tiles = (int*)malloc(sizeof(int) * MAX_TRIANGLES); // For tile-based rasterization optimization
    display->fill_tracker = (int*)malloc(sizeof(int) * ((WIDTH / TILE_SIZE) * (HEIGHT / TILE_SIZE))); // For tile-based rasterization optimization
    display->max_triangles = MAX_TRIANGLES; // For tile-based rasterization optimization



    memset(display->pixels, 0, sizeof(uint32_t) * WIDTH * HEIGHT);
    display->z_buffer = (float*)malloc(sizeof(float) * WIDTH * HEIGHT);
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        display->z_buffer[i] = 10000.0f; // Initialize z-buffer to farthest depth
    }
    return display;
}

/*
 * create_grid_floor
 * Generates a planar grid mesh centered at origin in XZ, at vertical level `y_level`.
 * - Allocates `mesh->Vertices`, `mesh->Vertex_normals`, `mesh->camera_vertices`,
 *   `mesh->projected_vertices`, `mesh->transformed_normals`, `mesh->shaded_colors`, and `mesh->indices`.
 * - Sets a simple checkerboard color pattern.
 * NOTE: Implementation created with assistance from an AI tool.
 */
void create_grid_floor(Mesh* mesh, int segments, float size, float y_level) {
    int cols = segments + 1;
    int vertex_count = cols * cols;
    mesh->Vertices = (Vertex*)malloc(sizeof(Vertex) * vertex_count);
    mesh->Vertex_normals = (Vector3*)malloc(sizeof(Vector3) * vertex_count);
    mesh->camera_vertices = (Vector3*)malloc(sizeof(Vector3) * vertex_count);
    mesh->projected_vertices = (Vertex*)malloc(sizeof(Vertex) * vertex_count);
    mesh->transformed_normals = (Vector3*)malloc(sizeof(Vector3) * vertex_count);
    mesh->shaded_colors = (uint32_t*)malloc(sizeof(uint32_t) * vertex_count);
    mesh->vertex_count = vertex_count;

    float half = size * 0.5f;
    float step = size / (float)segments;
    for (int z = 0; z <= segments; z++) {
        for (int x = 0; x <= segments; x++) {
            int idx = z * cols + x;
            mesh->Vertices[idx].position.x = -half + x * step;
            mesh->Vertices[idx].position.y = y_level;
            mesh->Vertices[idx].position.z = -half + z * step;
            if (((x + z) & 1) == 0) mesh->Vertices[idx].color = 0xFFCCCCCC; else mesh->Vertices[idx].color = 0xFF777777;
            mesh->Vertex_normals[idx] = (Vector3){0.0f, 1.0f, 0.0f};
        }
    }

    int index_count = segments * segments * 6;
    int* indices = (int*)malloc(sizeof(int) * index_count);
    int k = 0;
    for (int z = 0; z < segments; z++) {
        for (int x = 0; x < segments; x++) {
            int a = z * cols + x;
            int b = a + 1;
            int c = a + cols;
            int d = c + 1;
            // Winding chosen so normal points up when viewed from above
            indices[k++] = a; indices[k++] = c; indices[k++] = b;
            indices[k++] = b; indices[k++] = c; indices[k++] = d;
        }
    }
    mesh->indices = indices;
    mesh->index_count = index_count;
    mesh->rotation_angle = 0.0f;
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

    for (int i = 0; i < 7; i++) {
     
        if (display->rendered_text[i] != NULL) {
           
            SDL_Rect destination_rect;
            destination_rect.w = 0; destination_rect.h = 0 ; // Let SDL_QueryTexture fill these in
            SDL_QueryTexture(display->rendered_text[i], NULL, NULL, &destination_rect.w, &destination_rect.h);
            destination_rect.y = i * destination_rect.h; // Stack text vertically
            destination_rect.x = 0; // Left align
        
            SDL_RenderCopy(display->renderer, display->rendered_text[i], NULL, &destination_rect);
            
        }
    }

    
    
    SDL_RenderPresent(display->renderer);
}

uint32_t* get_pixels(Display* display) {
    return display->pixels;
}

void process_events(Display* display, bool* running, Mouse* mouse, Camera* camera, Keyboard* keyboard, Mesh* mesh, char* debug_info) {
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

    if (camera-> position.y < 0.0f) {
        camera->position.y = 0.0f; // Prevent going below the floor
    }
    
    if (keyboard->keyboard_state[SDL_SCANCODE_1]) { // 1 key
        load_obj("..\\assets\\teapot.obj", mesh, 0xFFFF0000); // Brown for AK
        snprintf(debug_info, 64, "Rendering teapot.obj");
    }

    if (keyboard->keyboard_state[SDL_SCANCODE_2]) { // 2 key
        load_obj("..\\assets\\bunny.obj", mesh, 0xFFFFFFFF); // White color for bunny
        snprintf(debug_info, 64, "Rendering bunny.obj");
    }

    if (keyboard->keyboard_state[SDL_SCANCODE_3]) { // 3 key
        load_obj("..\\assets\\air-jordan-1.obj", mesh, 0xFF0000FF); // Blue color for Shoes
        snprintf(debug_info, 64, "Rendering air-jordan-1.obj");
    }

    if (keyboard->keyboard_state[SDL_SCANCODE_4]) { // 4 key
        load_obj("..\\assets\\xyzrgb_dragon.obj", mesh, 0xFF00FF00); // Green color for dragon
        snprintf(debug_info, 64, "Rendering xyzrgb_dragon.obj");
    }


     
}

void cleanup_window(Display* display) {
    free(display->pixels);
    free(display->z_buffer);
    free (display->tiles);
    free (display->global_tiles);
    free (display->fill_tracker);

    TTF_CloseFont(display->text_renderer);
    SDL_DestroyTexture(display->texture);
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    SDL_Quit();
    free(display);
}


void update_fps_text(Display* display, int frames_per_second) {
    char fps_text[64];
    snprintf(fps_text, sizeof(fps_text), "FPS: %d", frames_per_second);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* text_surface = TTF_RenderText_Blended(display->text_renderer, fps_text, white);
    if (text_surface) {
        if (display->rendered_text[0]) {
            SDL_DestroyTexture(display->rendered_text[0]);
        }
        display->rendered_text[0] = SDL_CreateTextureFromSurface(display->renderer, text_surface);
        SDL_FreeSurface(text_surface);
    }
}

void update_debug_text(Display* display, const char* debug_info) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* debug_surface = TTF_RenderText_Blended(display->text_renderer, debug_info, white);
    if (debug_surface) {
        if (display->rendered_text[1]) {
            SDL_DestroyTexture(display->rendered_text[1]);
        }
        display->rendered_text[1] = SDL_CreateTextureFromSurface(display->renderer, debug_surface);
        SDL_FreeSurface(debug_surface);
    }
}

void key_info_text(Display* display, const char* key_info) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* key_surface = TTF_RenderText_Blended(display->text_renderer, key_info, white);
    if (key_surface) {
        if (display->rendered_text[2]) {
            SDL_DestroyTexture(display->rendered_text[2]);
        }
        display->rendered_text[2] = SDL_CreateTextureFromSurface(display->renderer, key_surface);
        SDL_FreeSurface(key_surface);
    }
}

void update_render_info_text(Display* display, const char* render_info) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* render_surface = TTF_RenderText_Blended(display->text_renderer, render_info, white);
    if (render_surface) {
        if (display->rendered_text[3]) {
            SDL_DestroyTexture(display->rendered_text[3]);
        }
        display->rendered_text[3] = SDL_CreateTextureFromSurface(display->renderer, render_surface);
        SDL_FreeSurface(render_surface);
    }
}

void update_face_count_text(Display* display, int face_count) {
    char face_text[64];
    snprintf(face_text, sizeof(face_text), "Faces: %d", face_count);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* face_surface = TTF_RenderText_Blended(display->text_renderer, face_text, white);
    if (face_surface) {
        if (display->rendered_text[4]) {
            SDL_DestroyTexture(display->rendered_text[4]);
        }
        display->rendered_text[4] = SDL_CreateTextureFromSurface(display->renderer, face_surface);
        SDL_FreeSurface(face_surface);
    }
}

void update_text(Display* display, int* current_time, int* last_time, int* frames_per_second, int face_count, const char* debug_info, const char* render_info) {
    *current_time = SDL_GetTicks();
    if (*current_time - *last_time >= 1000) {
        
        *last_time = *current_time;

        update_fps_text(display, *frames_per_second);
        key_info_text(display, "Press 1-4 to load different models");
        update_debug_text(display, debug_info);
        update_render_info_text(display, render_info);
        update_face_count_text(display, face_count);
    }
    
}
