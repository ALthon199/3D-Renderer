#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

#include "geometry.h"
#include "display.h"

// The Painter: Functions that draw into a block of pixels
// These functions are independent of SDL or any window library

void put_pixel(Display* display, Vertex v);
void draw_line(Display* display, Vertex v0, Vertex v1);
void draw_triangle(Display* display, Triangle tri);
void draw_mesh(Display* display, Mesh* mesh, Camera* camera);

#endif