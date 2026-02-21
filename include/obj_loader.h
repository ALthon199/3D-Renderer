#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "geometry.h"
#include "vector_math.h"
int load_obj(const char* filename, Mesh* mesh, uint32_t default_color);

#endif // OBJ_LOADER_H