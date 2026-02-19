#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdint.h>
#include <stdbool.h>


#define MAX_TRIANGLES 2000
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

    Gradient gradient; // Precomputed gradients for interpolation
} Triangle;


typedef struct{
    Triangle tri[MAX_TRIANGLES]; // 16x16 pixel tiles for tile-based rasterization optimization
    int tri_count;
} Tile;

typedef struct {
    Vertex* Vertices;
    Vector3* Vertex_normals;
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
