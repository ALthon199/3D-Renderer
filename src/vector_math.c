#include "vector_math.h"
#include <math.h>
#include "geometry.h"
#include "display.h"
#include "renderer.h"
Vector3 forward(Camera* camera) {
    float x = sinf(camera->yaw) * cosf(camera->pitch);
    float y = sinf(camera->pitch);
    float z = cosf(camera->yaw) * cosf(camera->pitch);
    return vec_normalize((Vector3){x, y, z});
}

Vector3 vec_sub(Vector3 a, Vector3 b){
    return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 vec_add(Vector3 a, Vector3 b){
    return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 vec_normalize(Vector3 v){
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0) {
        return (Vector3){0, 0, 0}; // Avoid division by zero
    }
    return (Vector3){v.x / length, v.y / length, v.z / length};
}

Vector3 vec_cross(Vector3 a, Vector3 b){
    return (Vector3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
Vertex camera_translate(Vertex v, Camera* camera) {
    return (Vertex){
        vec_sub(v.position, camera->position),
        v.color
    };
}
Vertex camera_rotate(Vertex v, Camera* camera) {
    float cos_yaw = cosf(camera->yaw);
    float sin_yaw = sinf(camera->yaw);
    float cos_pitch = cosf(camera->pitch);
    float sin_pitch = sinf(camera->pitch);

    // Rotate around Y axis (yaw)
    float x = v.position.x * cos_yaw - v.position.z * sin_yaw;
    float z = v.position.x * sin_yaw + v.position.z * cos_yaw;

    // Rotate around X axis (pitch)
    float y = v.position.y * cos_pitch - z * sin_pitch;
    z  = v.position.y * sin_pitch + z * cos_pitch;

    return (Vertex){(Vector3){x, y, z}, v.color};
}

Vertex camera_project(Vertex v) {
    float fov_factor = 600.0f;
    float z = v.position.z;
    if (z < 0.1f) z = 0.1f; // Clamp to avoid division-by-zero and extreme perspective when very near
    float x = (v.position.x / z) * fov_factor + (WIDTH / 2);
    float y = -(v.position.y / z) * fov_factor + (HEIGHT / 2);
    return (Vertex){(Vector3){x, y, z}, v.color};
}


float vec_dot(Vector3 a, Vector3 b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}




