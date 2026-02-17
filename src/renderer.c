#include "display.h"
#include "renderer.h"
#include "vector_math.h"
#include <math.h>
#include <stdio.h>
#include <omp.h>
// The cross product of a given points and two other points.
static float edge_function(float px, float py, float x0, float y0, float x1, float y1) {
    return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
}
uint32_t get_shaded_color(uint32_t base_color, float intensity) {
    uint8_t r = (uint8_t)(((base_color >> 16) & 0xFF) * intensity);
    uint8_t g = (uint8_t)(((base_color >> 8) & 0xFF) * intensity);
    uint8_t b = (uint8_t)(((base_color >> 0) & 0xFF) * intensity);
    return (0xFF << 24) | (r << 16) | (g << 8) | (b << 0);
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
    Vertex v0 = tri.v0; Vertex v1 = tri.v1; Vertex v2 = tri.v2;

    // 1. Bounding Box
    int minX = (int)fmaxf(0, floorf(fminf(v0.position.x, fminf(v1.position.x, v2.position.x))));
    int minY = (int)fmaxf(0, floorf(fminf(v0.position.y, fminf(v1.position.y, v2.position.y))));
    int maxX = (int)fminf(WIDTH - 1, ceilf(fmaxf(v0.position.x, fmaxf(v1.position.x, v2.position.x))));
    int maxY = (int)fminf(HEIGHT - 1, ceilf(fmaxf(v0.position.y, fmaxf(v1.position.y, v2.position.y))));

    float area = edge_function(v0.position.x, v0.position.y, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
    if (fabs(area) < 1e-6f) return;
    float inv_area = 1.0f / area;

    // 2. Precompute Gradients (Slopes)
    // How much does an attribute change if we move 1 pixel in X or Y?
    float w0_dx = v1.position.y - v0.position.y;
    float w1_dx = v2.position.y - v1.position.y;
    float w2_dx = v0.position.y - v2.position.y;
    float w0_dy = v0.position.x - v1.position.x;
    float w1_dy = v1.position.x - v2.position.x;
    float w2_dy = v2.position.x - v0.position.x;

    float dz_dx = (w0_dx * v0.position.z + w1_dx * v1.position.z + w2_dx * v2.position.z) * inv_area;
    float dz_dy = (w0_dy * v0.position.z + w1_dy * v1.position.z + w2_dy * v2.position.z) * inv_area;

    float r0 = (v0.color >> 16) & 0xFF, g0 = (v0.color >> 8) & 0xFF, b0 = v0.color & 0xFF;
    float r1 = (v1.color >> 16) & 0xFF, g1 = (v1.color >> 8) & 0xFF, b1 = v1.color & 0xFF;
    float r2 = (v2.color >> 16) & 0xFF, g2 = (v2.color >> 8) & 0xFF, b2 = v2.color & 0xFF;

    float dr_dx = (w0_dx * r0 + w1_dx * r1 + w2_dx * r2) * inv_area;
    float dr_dy = (w0_dy * r0 + w1_dy * r1 + w2_dy * r2) * inv_area;
    float dg_dx = (w0_dx * g0 + w1_dx * g1 + w2_dx * g2) * inv_area;
    float dg_dy = (w0_dy * g0 + w1_dy * g1 + w2_dy * g2) * inv_area;
    float db_dx = (w0_dx * b0 + w1_dx * b1 + w2_dx * b2) * inv_area;
    float db_dy = (w0_dy * b0 + w1_dy * b1 + w2_dy * b2) * inv_area;

    // 3. Initial values at the start of the bounding box (minX + 0.5, minY + 0.5)
    float w0_row = edge_function(minX + 0.5f, minY + 0.5f, v0.position.x, v0.position.y, v1.position.x, v1.position.y);
    float w1_row = edge_function(minX + 0.5f, minY + 0.5f, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
    float w2_row = edge_function(minX + 0.5f, minY + 0.5f, v2.position.x, v2.position.y, v0.position.x, v0.position.y);

    float z_row = (w0_row * v0.position.z + w1_row * v1.position.z + w2_row * v2.position.z) * inv_area;
    float r_row = (w0_row * r0 + w1_row * r1 + w2_row * r2) * inv_area;
    float g_row = (w0_row * g0 + w1_row * g1 + w2_row * g2) * inv_area;
    float b_row = (w0_row * b0 + w1_row * b1 + w2_row * b2) * inv_area;

    for (int j = minY; j <= maxY; j++) {
        float w0 = w0_row; float w1 = w1_row; float w2 = w2_row;
        float z = z_row; float r = r_row; float g = g_row; float b = b_row;

        for (int i = minX; i <= maxX; i++) {
            // Check if inside (inclusive of both CW and CCW for safety)
            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                int index = j * WIDTH + i;
                if (z < display->z_buffer[index]) {
                    display->z_buffer[index] = z;
                    display->pixels[index] = (0xFF << 24) | ((uint8_t)r << 16) | ((uint8_t)g << 8) | (uint8_t)b;
                }
            }
            // INCREMENT EVERYTHING
            w0 += w0_dx; w1 += w1_dx; w2 += w2_dx;
            z += dz_dx; r += dr_dx; g += dg_dx; b += db_dx;
        }
        w0_row += w0_dy; w1_row += w1_dy; w2_row += w2_dy;
        z_row += dz_dy; r_row += dr_dy; g_row += dg_dy; b_row += db_dy;
    }
}
void draw_mesh(Display* display, Mesh* mesh, Camera* camera) {


    float fov_factor = 600.0f;

    // Transform all vertices from world space to camera space, then project to screen space
    for (int i = 0; i < mesh->vertex_count; i += 1) {
        Vertex v0 = mesh->Vertices[i];
        Vector3 normal = mesh->Vertex_normals[i];
        // Rotate about y axis
        float angle = mesh->rotation_angle;
        float cos_a = cos(angle);
        float sin_a = sin(angle);
        
        // Rotate Vertex/Vertex Normals
        float rotated_x = (v0.position.x * cos_a - v0.position.z * sin_a);
        float rotated_y = v0.position.y; 
        float rotated_z = (v0.position.x * sin_a + v0.position.z * cos_a) + 2.0f; // Translate forward so it's in front of camera
        
        float rotated_xnormal = (normal.x * cos_a - normal.z * sin_a);
        float rotated_ynormal = normal.y;
        float rotated_znormal = (normal.x * sin_a + normal.z * cos_a);

        // Translate, rotate, and project vertex from world space to camera space
        Vertex translated_v = camera_translate((Vertex){(Vector3){rotated_x, rotated_y, rotated_z}, v0.color}, camera);
        Vertex rotated_v = camera_rotate(translated_v, camera);
        mesh->camera_vertices[i] = rotated_v.position;
        Vertex projected_v = camera_project(rotated_v);
        mesh->projected_vertices[i] = projected_v;


        // Rotate vertex normal to camera space for lighting calculations later
        mesh->transformed_normals[i] = (Vector3){rotated_xnormal, rotated_ynormal, rotated_znormal};
        mesh->transformed_normals[i] = camera_rotate((Vertex){mesh->transformed_normals[i], 0}, camera).position;
        mesh->transformed_normals[i] = vec_normalize(mesh->transformed_normals[i]);     
        
        // Gouraud shading: Calculate light intensity at vertex and store shaded color in mesh for use during rasterization
        Vector3 light_dir = vec_normalize((Vector3){1, 1, -1}); // Simple directional light
        float light_intensity = fmaxf(0.1f, vec_dot(mesh->transformed_normals[i], light_dir));
        uint32_t base_color = v0.color;
        uint32_t shaded_color = get_shaded_color(base_color, light_intensity);

        mesh->shaded_colors[i] = shaded_color;
    }
   
    int i;
    // Backface culling and triangle rasterization
    for (i = 0; i < mesh->index_count; i+=3){

        Vector3 camerav0 = mesh->camera_vertices[mesh->indices[i]];
        Vector3 camerav1 = mesh->camera_vertices[mesh->indices[i+1]];
        Vector3 camerav2 = mesh->camera_vertices[mesh->indices[i+2]];

        Vector3 triangle_facing = vec_cross(vec_sub(camerav1, camerav0), vec_sub(camerav2, camerav0));                             
        Vector3 camera_facing = camerav0; // Camera is at origin in camera space, so vector from triangle to camera is just camerav0
        
        float dot = triangle_facing.x * camera_facing.x + triangle_facing.y * camera_facing.y + triangle_facing.z * camera_facing.z;
        if (dot >= 0) {
            continue; 
        } 
       

        Vertex v0 = mesh->projected_vertices[mesh->indices[i]];
        Vertex v1 = mesh->projected_vertices[mesh->indices[i+1]];
        Vertex v2 = mesh->projected_vertices[mesh->indices[i+2]];
        
        
        if (v0.position.z < 0.001f || v1.position.z < 0.001f || v2.position.z < 0.001f) {
           // Don't draw triangles that are behind the camera or too close to it
            continue;
        }


        v0.color = mesh->shaded_colors[mesh->indices[i]];
        v1.color = mesh->shaded_colors[mesh->indices[i+1]];
        v2.color = mesh->shaded_colors[mesh->indices[i+2]];

        Vertex vertices[3] = {v0, v1, v2};
        Triangle tri = {vertices[0], vertices[1], vertices[2]};
        draw_triangle(display, tri);
    }

    
}

void free_mesh(Mesh* mesh) {
    free(mesh->Vertices);
    free(mesh->Vertex_normals);
    free(mesh->indices);
    free(mesh->camera_vertices);
    free(mesh->projected_vertices);
    free(mesh->transformed_normals);
    free(mesh->shaded_colors);
}