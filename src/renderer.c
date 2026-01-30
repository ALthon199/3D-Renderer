#include "renderer.h"


void put_pixel(uint32_t* pixels, int x, int y, uint32_t color){
    // colors the index of pixel at row X and col Y
    if (x < 0 || x >= WIDTH){
        return;
    }
    if (y < 0 || y >= HEIGHT){
        return;
    }

    int index = (y * WIDTH) + x;
    pixels[index] = color;
    
}

static void draw_line_high(uint32_t* pixels, int x0, int y0, int x1, int y1, uint32_t color){

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

static void draw_line_low(uint32_t* pixels, int x0, int y0, int x1, int y1, uint32_t color){

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

void draw_line(uint32_t* pixels, int x0, int y0, int x1, int y1, uint32_t color){
    
    if (abs(x0-x1) >= abs(y0-y1)){
        draw_line_low(pixels, x0, y0, x1, y1, color);
    }
    else{
        draw_line_high(pixels, x0, y0, x1, y1, color);
    }
}