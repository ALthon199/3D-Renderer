#include "display.h"
#include "renderer.h"
#include <math.h>
#include <stdio.h>
// The cross product of a given points and two other points.
static float edge_function(float px, float py, float x0, float y0, float x1, float y1) {
    return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
}

void put_pixel(Display* display, Vertex v) {
    // Snap float coordinates to integer pixel indices when coloring
    int ix = (int)(v.x);
    int iy = (int)(v.y);
    if (ix < 0 || ix >= WIDTH) {
        return;
    }
    if (iy < 0 || iy >= HEIGHT) {
        return;
    }

    
    int index = (iy * WIDTH) + ix;
    if (display->z_buffer[index] > v.z) {
        display->z_buffer[index] = v.z;
        display->pixels[index] = v.color;
    }
   
  
    
}
static void draw_line_high(Display* display, Vertex v0, Vertex v1, uint32_t color) {
    if (v0.y > v1.y) {
        Vertex temp = v0;
        v0 = v1;
        v1 = temp;
    }

    int dx = (int)roundf(v1.x) - (int)roundf(v0.x);
    int dy = (int)roundf(v1.y) - (int)roundf(v0.y);
    int x = (int)roundf(v0.x);
    int xi = 1;
    if (dx < 0) {
        dx = -dx;
        xi = -1;
    }
        
    int D = 2 * dx - dy;

    for (int y = (int)roundf(v0.y); y < (int)roundf(v1.y) + 1; y++) {
        put_pixel(display, (Vertex){(float)x, (float)y, v0.z, color});
        if (D > 0) {
            x = x + xi;
            D = D + 2 * (dx-dy);
        }
        else {
            D = D + 2 * dx;
        }
    }
}
static void draw_line_low(Display* display, Vertex v0, Vertex v1, uint32_t color) {
    if (v0.x > v1.x) {
        Vertex temp = v0;
        v0 = v1;
        v1 = temp;
    }

    int dx = (int)roundf(v1.x) - (int)roundf(v0.x);
    int dy = (int)roundf(v1.y) - (int)roundf(v0.y);
    int y = (int)roundf(v0.y);
    int yi = 1;
    
    if (dy < 0) {
        dy = -dy;
        yi = -1;
    }

    int D = 2 * dy - dx;
    for (int x = (int)roundf(v0.x); x < (int)roundf(v1.x) + 1; x++) {
        put_pixel(display, (Vertex){(float)x, (float)y, v0.z, color});
        if (D > 0) {
            y = y + yi;
            D = D + 2 * (dy-dx);
        }
        else {
            D = D + 2 * dy;
        }
    }
}
void draw_line(Display* display, Vertex v0, Vertex v1) {
    // Bresenham's line algorithm works with integer coordinates
    // Color is that of v0 for simplicity
    int x0 = (int)roundf(v0.x);
    int y0 = (int)roundf(v0.y);
    int x1 = (int)roundf(v1.x);
    int y1 = (int)roundf(v1.y);

    if (abs(x0-x1) >= abs(y0-y1)) {
        draw_line_low(display, v0, v1, v0.color);
    }
    else {
        draw_line_high(display, v0, v1, v0.color);
    }
}

void draw_triangle(Display* display, Triangle tri) {
    // Triangle rasterization using barycentric coordinates
    // Triangle vertexes have to be clockwise.

    // Compute bounding box in float, then iterate over integer pixel centers
    Vertex v0 = tri.v0;
    Vertex v1 = tri.v1;
    Vertex v2 = tri.v2;
    float minXf = fmaxf(0.0f, fminf(v0.x, fminf(v1.x, v2.x)));
    float minYf = fmaxf(0.0f, fminf(v0.y, fminf(v1.y, v2.y)));
    float maxXf = fminf((float)(WIDTH - 1), fmaxf(v0.x, fmaxf(v1.x, v2.x)));
    float maxYf = fminf((float)(HEIGHT - 1), fmaxf(v0.y, fmaxf(v1.y, v2.y)));

    int minX = (int)floorf(minXf);
    int minY = (int)floorf(minYf);
    int maxX = (int)ceilf(maxXf);
    int maxY = (int)ceilf(maxYf);

     
    // Precompute colors at vertices
    float r0 = (float)((v0.color >> 16) & 0xFF);
    float g0 = (float)((v0.color >> 8) & 0xFF);
    float b0 = (float)((v0.color >> 0) & 0xFF);
    float r1 = (float)((v1.color >> 16) & 0xFF);
    float g1 = (float)((v1.color >> 8) & 0xFF);
    float b1 = (float)((v1.color >> 0) & 0xFF);
    float r2 = (float)((v2.color >> 16) & 0xFF);
    float g2 = (float)((v2.color >> 8) & 0xFF);
    float b2 = (float)((v2.color >> 0) & 0xFF);

    // Precompute edge function values at top-left corner of bounding box
    // Sample from center of the square pixel
    float original_w0_row = edge_function((float)minX + 0.5f, (float)minY + 0.5f, v0.x, v0.y, v1.x, v1.y);
    float original_w1_row = edge_function((float)minX + 0.5f, (float)minY + 0.5f, v1.x, v1.y, v2.x, v2.y);
    float original_w2_row = edge_function((float)minX + 0.5f, (float)minY + 0.5f, v2.x, v2.y, v0.x, v0.y);

    float w0_dx = v1.y - v0.y;
    float w1_dx = v2.y - v1.y;
    float w2_dx = v0.y - v2.y;

    float w0_dy = v0.x - v1.x;
    float w1_dy = v1.x - v2.x;
    float w2_dy = v2.x - v0.x;
    
    float area = edge_function(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);
    float inv_area = 1.0f / area;

    // Early out if the triangle has zero or positive area
    if (fabsf(area) < 1e-6f || area > 0.0f) {
        return;
    }

    for (int j = minY; j <= maxY; j++) {
        float w0_col = original_w0_row;
        float w1_col = original_w1_row;
        float w2_col = original_w2_row;

        for (int i = minX; i <= maxX; i++) {
            // If the pixel is inside the triangle or on its edge then shade
            if (w0_col <= 0.0f && w1_col <= 0.0f && w2_col <= 0.0f) {
                
                float weight0 = w0_col * inv_area;
                float weight1 = w1_col * inv_area;
                float weight2 = w2_col * inv_area;
                
                float depth = (weight0 * v0.z) + (weight1 * v1.z)  + (weight2 * v2.z);
                
                uint8_t a = (uint8_t)(0xFF);
                uint8_t r = (uint8_t)(r0 * weight0 + r1 * weight1 + r2 * weight2);
                uint8_t g = (uint8_t)(g0 * weight0 + g1 * weight1 + g2 * weight2);
                uint8_t b = (uint8_t)(b0 * weight0 + b1 * weight1 + b2 * weight2);

                uint32_t color = (a << 24) | (r << 16) | (g << 8) | (b << 0);

                // Only place if closer to camera
                put_pixel(display, (Vertex){(float)(i+0.5f) , (float)(j+0.5f) , depth, color});
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

void draw_mesh(Display* display, Mesh* mesh) {

    Vertex rotated_vertices[8];

    float fov_factor = 600.0f;
    
    for (int i = 0; i < 8; i += 1) {
        Vertex v0 = mesh->Vertices[i];
        // Rotate about y axis
        float angle = mesh->rotation_angle;
        float x = v0.x * cos(angle) - v0.z * sin(angle);
        float y = v0.y;
        float z = v0.x * sin(angle) + v0.z * cos(angle) + mesh->translation;

        x = (x/z) * fov_factor + WIDTH / 2;
        y = (y/z) * fov_factor + HEIGHT / 2;
        Vertex newV = {x, y, z, v0.color};
        
        rotated_vertices[i] = newV;
       
    }

    for (int i = 0; i < mesh->index_count; i += 3){
        Vertex v0 = rotated_vertices[mesh->indices[i]];
        Vertex v1 = rotated_vertices[mesh->indices[i+1]];
        Vertex v2 = rotated_vertices[mesh->indices[i+2]];
        Triangle tri = {v0, v1, v2};
        draw_triangle(display, tri);
    }

    
}