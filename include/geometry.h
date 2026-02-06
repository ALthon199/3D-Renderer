#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdint.h>
#include <stdbool.h>

// Vertex: Stores x, y position and color
typedef struct {
    float x, y, z;
    uint32_t color;
    
} Vertex;

typedef struct{
    Vertex v0, v1, v2;
} Triangle;


typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    Vertex* Vertices;
    int* indices;
    int index_count;

    float rotation_angle;
    float translation;
} Mesh;

#endif
