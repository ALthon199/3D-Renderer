#ifndef MESH_UTILS_H
#define MESH_UTILS_H
#include "geometry.h"
#include "renderer.h"
#include "vector_math.h"

void transform_mesh(Display* display, Mesh* mesh, Camera* camera);



void prefix_sum_tiles(Tile* tiles, int total_tiles);


// Allocates and fills a global pool of triangle indices for each tile, as well as render each tile's triangles in parallel
// Returns total number of triangles rendered
int render_mesh(Display* display, Mesh* mesh, Camera* camera, Tile* tiles);




#endif