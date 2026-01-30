#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define HEIGHT 800
#define WIDTH 1200

typedef struct{
    int mouseX;
    int mouseY;
    bool pressed;
} Mouse;

void put_pixel(uint32_t* pixels, int x, int y, uint32_t color);
void draw_line(uint32_t* pixels, int x0, int y0, int x1, int y1, uint32_t color);

#endif