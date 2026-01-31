#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define HEIGHT 800
#define WIDTH 1200
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
typedef struct{
    int mouseX;
    int mouseY;
    bool pressed;
} Mouse;


void draw_rect(uint32_t* pixels, int x, int y, int w, int h, uint32_t color);

int edge_function(int x0, int y0, int x1, int y1, int x2, int y2);
void draw_triangle(uint32_t* pixels, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void clear_screen(uint32_t* pixels, uint32_t color);
void put_pixel(uint32_t* pixels, int x, int y, uint32_t color);
void draw_line(uint32_t* pixels, int x0, int y0, int x1, int y1, uint32_t color);








#endif