#include "display.h"
#include "renderer.h"
#include "vector_math.h"
#include <math.h>
#include <stdio.h>

// The cross product of a given points and two other points.
static float edge_function(float px, float py, float x0, float y0, float x1, float y1) {
    return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
}

void put_pixel(Display* display, Vertex v) {
    // Snap float coordinates to integer pixel indices when coloring
    int ix = (int)(v.position.x);
    int iy = (int)(v.position.y);
    if (ix < 0 || ix >= WIDTH) {
        return;
    }
    if (iy < 0 || iy >= HEIGHT) {
        return;
    }

    
    int index = (iy * WIDTH) + ix;
    
    if (display->z_buffer[index] >  v.position.z) {
        display->z_buffer[index] = v.position.z;
        display->pixels[index] = v.color;
    }
   
  
    
}
static void draw_line_high(Display* display, Vertex v0, Vertex v1, uint32_t color) {
    if (v0.position.y > v1.position.y) {
        Vertex temp = v0;
        v0 = v1;
        v1 = temp;
    }

    int dx = (int)roundf(v1.position.x) - (int)roundf(v0.position.x);
    int dy = (int)roundf(v1.position.y) - (int)roundf(v0.position.y);
    int x = (int)roundf(v0.position.x);
    int xi = 1;
    if (dx < 0) {
        dx = -dx;
        xi = -1;
    }
        
    int D = 2 * dx - dy;

    for (int y = (int)roundf(v0.position.y); y < (int)roundf(v1.position.y) + 1; y++) {
        put_pixel(display, (Vertex){(Vector3){(float)x, (float)y, v0.position.z}, color});
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
    if (v0.position.x > v1.position.x) {
        Vertex temp = v0;
        v0 = v1;
        v1 = temp;
    }

    int dx = (int)roundf(v1.position.x) - (int)roundf(v0.position.x);
    int dy = (int)roundf(v1.position.y) - (int)roundf(v0.position.y);
    int y = (int)roundf(v0.position.y);
    int yi = 1;
    
    if (dy < 0) {
        dy = -dy;
        yi = -1;
    }

    int D = 2 * dy - dx;
    for (int x = (int)roundf(v0.position.x); x < (int)roundf(v1.position.x) + 1; x++) {
        put_pixel(display, (Vertex){(Vector3){(float)x, (float)y, v0.position.z}, color});
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
    int x0 = (int)roundf(v0.position.x);
    int y0 = (int)roundf(v0.position.y);
    int x1 = (int)roundf(v1.position.x);
    int y1 = (int)roundf(v1.position.y);

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
    float minXf = fmaxf(0.0f, fminf(v0.position.x, fminf(v1.position.x, v2.position.x)));
    float minYf = fmaxf(0.0f, fminf(v0.position.y, fminf(v1.position.y, v2.position.y)));
    float maxXf = fminf((float)(WIDTH - 1), fmaxf(v0.position.x, fmaxf(v1.position.x, v2.position.x)));
    float maxYf = fminf((float)(HEIGHT - 1), fmaxf(v0.position.y, fmaxf(v1.position.y, v2.position.y)));

    int minX = (int)floor(minXf);
    int minY = (int)floor(minYf);
    int maxX = (int)ceil(maxXf);
    int maxY = (int)ceil(maxYf);

     
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
    float original_w0_row = edge_function((float)minX + 0.5f, (float)minY + 0.5f, v0.position.x, v0.position.y, v1.position.x, v1.position.y);
    float original_w1_row = edge_function((float)minX + 0.5f, (float)minY + 0.5f, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
    float original_w2_row = edge_function((float)minX + 0.5f, (float)minY + 0.5f, v2.position.x, v2.position.y, v0.position.x, v0.position.y);

    float w0_dx = v1.position.y - v0.position.y;
    float w1_dx = v2.position.y - v1.position.y;
    float w2_dx = v0.position.y - v2.position.y;

    float w0_dy = v0.position.x - v1.position.x;
    float w1_dy = v1.position.x - v2.position.x;
    float w2_dy = v2.position.x - v0.position.x;
    
    float area = edge_function(v0.position.x, v0.position.y, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
    float inv_area = 1.0f / area;

    // Early out if the triangle has zero or positive area
    if (fabs(area) < 1e-6f) {
    
        return;
    }

    for (int j = minY; j <= maxY; j++) {
        float w0_col = original_w0_row;
        float w1_col = original_w1_row;
        float w2_col = original_w2_row;

        for (int i = minX; i <= maxX; i++) {
            
            // If the pixel is inside the triangle or on its edge then shade
            if ((w0_col <= 0.0f && w1_col <= 0.0f && w2_col <= 0.0f) 
            || (w0_col >= 0.0f && w1_col >= 0.0f && w2_col >= 0.0f)) {
                
                float weight0 = w0_col * inv_area;
                float weight1 = w1_col * inv_area;
                float weight2 = w2_col * inv_area;
                
                float depth = (weight0 * v0.position.z) + (weight1 * v1.position.z)  + (weight2 * v2.position.z);
                
                uint8_t a = (uint8_t)(0xFF);
                uint8_t r = (uint8_t)(r0 * weight0 + r1 * weight1 + r2 * weight2);
                uint8_t g = (uint8_t)(g0 * weight0 + g1 * weight1 + g2 * weight2);
                uint8_t b = (uint8_t)(b0 * weight0 + b1 * weight1 + b2 * weight2);

                uint32_t color = (a << 24) | (r << 16) | (g << 8) | (b << 0);
                

                // Only place if closer to camera
                put_pixel(display, (Vertex){(Vector3){(float)(i+0.5f), (float)(j+0.5f), depth}, color});
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

void draw_mesh(Display* display, Mesh* mesh, Camera* camera) {


    float fov_factor = 600.0f;
    
    for (int i = 0; i < mesh->vertex_count; i += 1) {
        Vertex v0 = mesh->Vertices[i];
        
        // Rotate about y axis
        float angle = mesh->rotation_angle;
        float cos_a = cos(angle);
        float sin_a = sin(angle);
        

        float rotated_x = (v0.position.x * cos_a - v0.position.z * sin_a);
        float rotated_y = v0.position.y; 
        float rotated_z = (v0.position.x * sin_a + v0.position.z * cos_a) + 5.0f; // Translate forward so it's in front of camera
        
        Vertex translated_v = camera_translate((Vertex){(Vector3){rotated_x, rotated_y, rotated_z}, v0.color}, camera);
        Vertex rotated_v = camera_rotate(translated_v, camera);
        mesh->camera_vertices[i] = rotated_v.position;
    

       Vertex projected_v = camera_project(rotated_v);
       mesh->projected_vertices[i] = projected_v;
        
       
    }

    for (int i = 0; i < mesh->index_count; i += 3){
        Vector3 camerav0 = mesh->camera_vertices[mesh->indices[i]];
        Vector3 camerav1 = mesh->camera_vertices[mesh->indices[i+1]];
        Vector3 camerav2 = mesh->camera_vertices[mesh->indices[i+2]];

        Vector3 triangle_facing = vec_cross(vec_sub(camerav2, camerav0), vec_sub(camerav1, camerav0));
                                        
        Vector3 camera_facing = camerav0; // Camera is at origin in camera space, so vector from triangle to camera is just camerav0
        
        float dot = triangle_facing.x * camera_facing.x + triangle_facing.y * camera_facing.y + triangle_facing.z * camera_facing.z;
        if (dot <= 0) {
            continue; 
        } 
       
        Vertex v0 = mesh->projected_vertices[mesh->indices[i]];
        Vertex v1 = mesh->projected_vertices[mesh->indices[i+1]];
        Vertex v2 = mesh->projected_vertices[mesh->indices[i+2]];
        
        Triangle tri = {v0, v1, v2};

        
        if (v0.position.z < 0.001f || v1.position.z < 0.001f || v2.position.z < 0.001f) {
           // Don't draw triangles that are behind the camera or too close to it
            continue;
        }
        
        // Red for back faces
        draw_triangle(display, tri);
    }

    
}