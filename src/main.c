#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "renderer.h"
#define HEIGHT 800
#define WIDTH 1200



int main(int argc, char* argv[]) {
   
    SDL_Init(SDL_INIT_VIDEO);

    
    SDL_Window* window = SDL_CreateWindow("Test", 100, 100, WIDTH , HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    Mouse window_mouse = {0};
    



    uint32_t* pixels = malloc(sizeof(uint32_t) * WIDTH * HEIGHT);
    memset(pixels, 0, sizeof(uint32_t) * WIDTH * HEIGHT);

    
    
    uint32_t white = 0xFFFFFFFF;
    uint32_t color = 0xFFFF0000;
    

    int old_mouseX = 0;
    int old_mouseY = 0;
    SDL_GetMouseState(&window_mouse.mouseX, &window_mouse.mouseY);
    SDL_GetMouseState(&old_mouseX, &old_mouseY);
    
  
 
    SDL_Event e;
    bool running = true;
    while (running) {
        
        old_mouseX = window_mouse.mouseX;
        old_mouseY = window_mouse.mouseY;
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                running = false;
            }

            if (e.type == SDL_MOUSEMOTION){
                
                window_mouse.mouseX = e.motion.x;
                window_mouse.mouseY = e.motion.y;
                
            }

            if (e.type == SDL_MOUSEBUTTONDOWN){
                window_mouse.pressed = true;
            }
            if (e.type == SDL_MOUSEBUTTONUP){
                window_mouse.pressed = false;
            }
            
            //Clears screen if 'C' is pressed
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == 6){
                clear_screen(pixels, white);
            }
            

        }
        

        if (window_mouse.pressed){
            draw_line(pixels, old_mouseX, old_mouseY, window_mouse.mouseX, window_mouse.mouseY, 0xFFFF0000);
        }
        
        draw_triangle(pixels, 100, 100, 600, 100, 300, 500, 0xFFFF0000);
        
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(10);

    }
    free(pixels);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}