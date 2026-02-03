#include "renderer.h"
#include <math.h>


void put_pixel(uint32_t* pixels, float x, float y, uint32_t color){
    // Snap float coordinates to integer pixel indices when coloring
    int ix = (int)(x);
    int iy = (int)(y);
    if (ix < 0 || ix >= WIDTH){
        return;
    }
    if (iy < 0 || iy >= HEIGHT){
        return;
    }

    int index = (iy * WIDTH) + ix;
    pixels[index] = color;

}
void clear_screen(uint32_t* pixels, uint32_t color){
    memset(pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));
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
        put_pixel(pixels, (float)x, (float)y, color);
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
        put_pixel(pixels, (float)x, (float)y, color);
        if (D > 0) {
            y = y + yi;
            D = D + 2 * (dy-dx);
        }
        else{
            D = D + 2 * dy;
        }
    }
   
}
void draw_line(uint32_t* pixels, float x0f, float y0f, float x1f, float y1f, uint32_t color){
    // Bresenham's line algorithm works with integer coordinates
    int x0 = (int)roundf(x0f);
    int y0 = (int)roundf(y0f);
    int x1 = (int)roundf(x1f);
    int y1 = (int)roundf(y1f);

    if (abs(x0-x1) >= abs(y0-y1)){
        draw_line_low(pixels, x0, y0, x1, y1, color);
    }
    else{
        draw_line_high(pixels, x0, y0, x1, y1, color);
    }
}
void draw_rect(uint32_t* pixels, float x0, float y0, float width, float height, uint32_t color){


    for (int i = 0; i < (int)width; i++){
        for (int j = 0; j < (int)height; j++){
            put_pixel(pixels, x0 + (float)i, y0 + (float)j, color);
        }
    }
}

float edge_function(float px, float py, float x0, float y0, float x1, float y1){
    return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
}

void draw_triangle(uint32_t* pixels, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t c0, uint32_t c1, uint32_t c2){

    // Triangle rasterization using barycentric coordinates

    // Compute bounding box in float, then iterate over integer pixel centers
    float minXf = fmaxf(0.0f, fminf(x0, fminf(x1, x2)));
    float minYf = fmaxf(0.0f, fminf(y0, fminf(y1, y2)));
    float maxXf = fminf((float)(WIDTH - 1), fmaxf(x0, fmaxf(x1, x2)));
    float maxYf = fminf((float)(HEIGHT - 1), fmaxf(y0, fmaxf(y1, y2)));

    int minX = (int)floorf(minXf);
    int minY = (int)floorf(minYf);
    int maxX = (int)ceilf(maxXf);
    int maxY = (int)ceilf(maxYf);
    

    // Ensure pixels are within bounds
    if (minX < 0 || minY < 0 || maxX >= WIDTH || maxY >= HEIGHT){
        printf("Triangle bounding box out of bounds: (%d, %d) to (%d, %d)\n", minX, minY, maxX, maxY);
        return;
    }
    // Precompute colors at vertices
    float r0 = (float)((c0 >> 16) & 0xFF);
    float g0 = (float)((c0 >> 8) & 0xFF);
    float b0 = (float)((c0 >> 0) & 0xFF);
    float r1 = (float)((c1 >> 16) & 0xFF);
    float g1 = (float)((c1 >> 8) & 0xFF);
    float b1 = (float)((c1 >> 0) & 0xFF);
    float r2 = (float)((c2 >> 16) & 0xFF);
    float g2 = (float)((c2 >> 8) & 0xFF);
    float b2 = (float)((c2 >> 0) & 0xFF);
    // Precompute edge function values at top-left corner of bounding box
    float original_w0_row = edge_function((float)minX, (float)minY, x0, y0, x1, y1);
    float original_w1_row = edge_function((float)minX, (float)minY, x1, y1, x2, y2);
    float original_w2_row = edge_function((float)minX, (float)minY, x2, y2, x0, y0);

    float w0_dx = y1 - y0;
    float w1_dx = y2 - y1;
    float w2_dx = y0 - y2;

    float w0_dy = x0 - x1;
    float w1_dy = x1 - x2;
    float w2_dy = x2 - x0;
    
    float area = edge_function(x0, y0, x1, y1, x2, y2);
    float inv_area = 1.0f / area;

    // Early out if the triangle has zero area
    if (fabsf(area) < 1e-6f){
        return;
    }

    for (int j = minY; j <= maxY; j++){
        float w0_col = original_w0_row;
        float w1_col = original_w1_row;
        float w2_col = original_w2_row;

        for (int i = minX; i <= maxX; i++){
            // If the pixel is inside the triangle or on its edge then shade
            if (w0_col <= 0.0f && w1_col <= 0.0f && w2_col <= 0.0f){
                float weight0 = w0_col * inv_area;
                float weight1 = w1_col * inv_area;
                float weight2 = w2_col * inv_area;

                uint8_t a = (uint8_t)(0xFF);
                uint8_t r = (uint8_t)(r0 * weight0 + r1 * weight1 + r2 * weight2);
                uint8_t g = (uint8_t)(g0 * weight0 + g1 * weight1 + g2 * weight2);
                uint8_t b = (uint8_t)(b0 * weight0 + b1 * weight1 + b2 * weight2);

                uint32_t color = (a << 24) | (r << 16) | (g << 8) | (b << 0);

                // Inline put_pixel for performance
                pixels[j * WIDTH + i] = color;
            }

            w0_col += w0_dx;
            w1_col += w1_dx;     
            w2_col += w2_dx;
        }

        original_w0_row += w0_dy;
        original_w1_row += w1_dy;
        original_w2_row += w2_dy;
    }
}

