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
    

    float old_mouseX = 0;
    float old_mouseY = 0;
    int tmpX = 0;
    int tmpY = 0;
    SDL_GetMouseState(&tmpX, &tmpY);
    window_mouse.mouseX = (float)tmpX;
    window_mouse.mouseY = (float)tmpY;
    old_mouseX = window_mouse.mouseX;
    old_mouseY = window_mouse.mouseY;
    
  
 
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
                window_mouse.mouseX = (float)e.motion.x;
                window_mouse.mouseY = (float)e.motion.y;
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

            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == 7){
                for (int i = 0; i < 1000; i++){
                    float x0 = (float)(rand() % WIDTH);
                    float y0 = (float)(rand() % HEIGHT);
                    float x1 = (float)(rand() % WIDTH);
                    float y1 = (float)(rand() % HEIGHT);
                    float x2 = (float)(rand() % WIDTH);
                    float y2 = (float)(rand() % HEIGHT);
                    uint32_t c0 = rand() % 0xFFFFFF;
                    uint32_t c1 = rand() % 0xFFFFFF;
                    uint32_t c2 = rand() % 0xFFFFFF;
                    draw_triangle(pixels, x0, y0, x1, y1, x2, y2, c0, c1, c2);
                }
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == 8){
                draw_triangle(pixels, window_mouse.mouseX, window_mouse.mouseY, 200.0f, 300.0f, 400.0f, 150.0f, 0xFFFF0000, 0xFF00FF00, 0xFF0000FF);
            }


            

        }
        

        if (window_mouse.pressed){
            draw_line(pixels, old_mouseX, old_mouseY, window_mouse.mouseX, window_mouse.mouseY, 0xFFFF0000);
        }
        
        
        
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