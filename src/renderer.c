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
void clear_screen(uint32_t* pixels, uint32_t color){
    for (int i = 0; i < WIDTH; i++){
        for (int j = 0;  j < HEIGHT; j++){
            put_pixel(pixels, i, j, color);
        }
    }
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
void draw_rect(uint32_t* pixels, int x0, int y0, int width, int height, uint32_t color){
    
    for (int i = 0; i < width; i ++){
        draw_line(pixels, i + x0, y0, i+x0, y0+height, color);
    }
}



int edge_function(int x, int y, int x0, int y0, int x1, int y1){

    // if > 0: point(x,y) is on the right of (x0,y0) -> (x1, y1)
    // if < 0: point(x,y) is on the left of (x0,y0) -> (x1, y1)
    // == 0:  point(x,y) is on (x0,y0) -> (x1, y1)
    return (x - x0) * (y1 - y0) - (y - y0) * (x1 - x0);
}

void draw_triangle(uint32_t* pixels, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color){
    // Draws triangle if point is within bounded triangle.
    int minX = MAX(0, MIN(x0, MIN(x1, x2)));
    int minY = MAX(0, MIN(y0, MIN(y1, y2)));
    int maxX = MIN(WIDTH - 1, MAX(x0, MAX(x1, x2)));
    int maxY = MIN(HEIGHT - 1, MAX(y0, MAX(y1, y2)));
    int deltaY;
    int deltaX;


    int original_w0_row = edge_function(minX, minY, x0, y0, x1, y1);
    int original_w1_row = edge_function(minX, minY, x1, y1, x2, y2);
    int original_w2_row = edge_function(minX, minY, x2, y2, x0, y0);

    int w0_row;
    int w1_row;
    int w2_row;
    for (int i = minX; i <= maxX; i++){
        
        w0_row = original_w0_row;
        w1_row = original_w1_row;
        w2_row = original_w2_row;

        for (int j = minY; j <= maxY; j++){
            if (w0_row <= 0 && w1_row <= 0 && w2_row <= 0){
                put_pixel(pixels, i, j, color);
            }
            w0_row -= (x1-x0);
            w1_row -= (x2-x1);
            w2_row -= (x0-x2);

            

        }
        original_w0_row += (y1-y0);
        original_w1_row += (y2-y1);
        original_w2_row += (y0-y2);
        

    }
}
