#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define HEIGHT 800
#define WIDTH 1200

void put_pixel(uint32_t* pixels, int x, int y, int color){
    // colors the index of pixel at row X and col Y

    int index = (y * WIDTH) + x;
    pixels[index] = color;
    
}

void draw_line_high(uint32_t* pixels, int x0, int y0, int x1, int y1, int color){

    if (y0 > y1){
        
        int temp_x = x0;
        int temp_y = y0;
        x0 = x1;
        y0 = y1;
        x1 = temp_x;
        y1 = temp_y;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int x = x0;
    int xi = 1;
    if (dx < 0){
        dx = -dx;
        xi = -1;
    }
        
    int D = 2 * dx - dy;

    for (int y = y0; y < y1 + 1; y++ ){
        put_pixel(pixels, x, y, color);
        if (D > 0) {
            x = x + xi;
            D = D + 2 * (dx-dy);
        }
        else{
            D = D + 2 * dx;
        }
    }
   
}

void draw_line_low(uint32_t* pixels, int x0, int y0, int x1, int y1, int color){

    if (x0 > x1){
        
        int temp_x = x0;
        int temp_y = y0;
        x0 = x1;
        y0 = y1;
        x1 = temp_x;
        y1 = temp_y;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int y = y0;
    int yi = 1;
    
    if (dy < 0){
        dy = -dy;
        yi = -1;
    }

    int D = 2 * dy - dx;
    for (int x = x0; x < x1 + 1; x++ ){
        put_pixel(pixels, x, y, color);
        if (D > 0) {
            y = y + yi;
            D = D + 2 * (dy-dx);
        }
        else{
            D = D + 2 * dy;
        }
    }
   
}
void draw_line(uint32_t* pixels, int x0, int y0, int x1, int y1, int color){
    printf("%s\n", "correct");
    if (abs(x0-x1) >= abs(y0-y1)){
        draw_line_low(pixels, x0, y0, x1, y1, color);
    }
    else{
        draw_line_high(pixels, x0, y0, x1, y1, color);
    }
}
int main(int argc, char* argv[]) {
   
    SDL_Init(SDL_INIT_VIDEO);

    
    SDL_Window* window = SDL_CreateWindow("Test", 100, 100, WIDTH , HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);


    uint32_t* pixels = malloc(sizeof(uint32_t) * WIDTH * HEIGHT);
    memset(pixels, 127, sizeof(uint32_t) * WIDTH * HEIGHT);

    for (int i = 0; i < WIDTH / 2; i++){
            for (int j = 0; j < HEIGHT/2; j++){
                
                put_pixel(pixels, (WIDTH / 2) + i, (HEIGHT / 2) + j, 0xFFF0FFF0);
                
            }
        }
    
    uint32_t color = 0xFFFF0000;
    

    SDL_Event e;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                running = false;
            }

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