#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <math.h>
#include "geometry.h"
#include "display.h"
Vector3 forward(Camera* camera);

Vector3 vec_sub(Vector3 a, Vector3 b);

Vector3 vec_add(Vector3 a, Vector3 b);

Vector3 vec_normalize(Vector3 v);

Vector3 vec_cross(Vector3 a, Vector3 b);

float vec_dot(Vector3 a, Vector3 b);

Vertex camera_translate(Vertex v, Camera* camera);

Vertex camera_rotate(Vertex v, Camera* camera);

Vertex camera_project(Vertex v);





#endif