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
    float mouseX;
    float mouseY;
    bool pressed;
} Mouse;

typedef struct{
    float x, y;
} Vertex;

void draw_rect(uint32_t* pixels, float x, float y, float w, float h, uint32_t color);

float edge_function(float px, float py, float x0, float y0, float x1, float y1);
void draw_triangle(uint32_t* pixels, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t c0, uint32_t c1, uint32_t c2);
void clear_screen(uint32_t* pixels, uint32_t color);
void put_pixel(uint32_t* pixels, float x, float y, uint32_t color);
void draw_line(uint32_t* pixels, float x0, float y0, float x1, float y1, uint32_t color);








#endif