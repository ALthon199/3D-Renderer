#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdint.h>
#include <stdbool.h>



#define FIXED_POINT_SHIFT 16
typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    Vector3 position;
    uint32_t color;
    
} Vertex;
typedef struct{
    float dz_dx, dz_dy;
    int dr_dx, dr_dy;
    int dg_dx, dg_dy;
    int db_dx, db_dy;
} Gradient;

typedef struct{
    Vertex v0, v1, v2;
    // Precomputed gradients for interpolation
    Gradient gradient; 

    // Bounding box for the triangle
    float minX, maxX, minY, maxY;

    // Precomputed values for edge function derivatives
    float w0_dx, w0_dy;
    float w1_dx, w1_dy;
    float w2_dx, w2_dy;

    // Precomputed row start, color start, and depth start for rasterization
    float w0_start; float w1_start; float w2_start;
    float z_start; int r_start; int g_start; int b_start;

    float inv_area;
} Triangle;


typedef struct{
    int offset; // 16x16 pixel tiles for tile-based rasterization optimization
    int tri_count;
} Tile;

typedef struct {
    Vertex* Vertices;
    Vector3* Vertex_normals;
    Triangle* rendered_triangles;
    int vertex_count;
    int* indices;
    int index_count;


    

    uint32_t* shaded_colors;
    Vector3* transformed_normals;
    Vector3* camera_vertices;
    Vertex* projected_vertices;
    float rotation_angle;
    float translation;
} Mesh;


#endif
