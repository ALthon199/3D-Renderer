#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

#include "geometry.h"
#include "display.h"

// The Painter: Functions that draw into a block of pixels
// These functions are independent of SDL or any window library
uint32_t get_shaded_color(uint32_t base_color, float intensity);
float edge_function(float px, float py, float x0, float y0, float x1, float y1);
void put_pixel(Display* display, Vertex v);
void draw_line(Display* display, Vertex v0, Vertex v1);
void draw_triangle(Display* display, Triangle* tri);
void draw_triangle_bounded(Display* display, Triangle tri, int minX, int minY, int maxX, int maxY);
void draw_mesh(Display* display, Mesh* mesh, Camera* camera);

#endif