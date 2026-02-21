#include "obj_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_obj(const char* filename, Mesh* mesh, uint32_t default_color) {


    FILE* obj = fopen(filename, "r");
    if (obj == NULL) {
        printf("Failed to open OBJ file: %s\n", filename);
        return -1;
    }

    int total_vertex = 0;
    int indices_est = 0;
    char line[512];

    // First pass: count vertices and face indices (triangular faces assumed)
    while (fgets(line, sizeof(line), obj)) {
        if (line[0] == 'v' && line[1] == ' ') total_vertex++;
        else if (line[0] == 'f' && line[1] == ' ') indices_est += 3;
    }

    if (total_vertex == 0) {
        printf("OBJ file has no vertices: %s\n", filename);
        fclose(obj);
        return -1;
    }

    fseek(obj, 0, SEEK_SET);

    Vertex* vertices = (Vertex*)malloc(sizeof(Vertex) * total_vertex);
    if (!vertices) {
        printf("Out of memory allocating vertices\n");
        fclose(obj);
        return -1;
    }

    int* vertex_indices = (int*)malloc(sizeof(int) * (indices_est > 0 ? indices_est : 1));
    if (!vertex_indices) {
        printf("Out of memory allocating indices\n");
        free(vertices);
        fclose(obj);
        return -1;
    }

    // Second pass: parse vertices and faces
    float vx = 0.0f, vy = 0.0f, vz = 0.0f;
    int index1 = 0, index2 = 0, index3 = 0;
    int vcount = 0;
    int icount = 0; // actual parsed indices

    while (fgets(line, sizeof(line), obj)) {
        if (line[0] == 'v' && line[1] == ' ') {
            if (sscanf(line, "v %f %f %f", &vx, &vy, &vz) == 3) {
                vertices[vcount++] = (Vertex){{vx, vy, vz}, 0xFFFF0000};
            }
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            int v[3]; int vt[3]; int vn[3];
            // face formats: v/vt/vn , v//vn , v/vt , v v v
            if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2]) == 9) {
                vertex_indices[icount++] = (v[0] > 0) ? v[0]-1 : total_vertex + v[0];
                vertex_indices[icount++] = (v[1] > 0) ? v[1]-1 : total_vertex + v[1];
                vertex_indices[icount++] = (v[2] > 0) ? v[2]-1 : total_vertex + v[2];
            }
            else if (sscanf(line, "f %d//%d %d//%d %d//%d", &v[0], &vn[0], &v[1], &vn[1], &v[2], &vn[2]) == 6) {
                vertex_indices[icount++] = (v[0] > 0) ? v[0]-1 : total_vertex + v[0];
                vertex_indices[icount++] = (v[1] > 0) ? v[1]-1 : total_vertex + v[1];
                vertex_indices[icount++] = (v[2] > 0) ? v[2]-1 : total_vertex + v[2];
            }
            else if (sscanf(line, "f %d/%d %d/%d %d/%d", &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2]) == 6) {
                vertex_indices[icount++] = (v[0] > 0) ? v[0]-1 : total_vertex + v[0];
                vertex_indices[icount++] = (v[1] > 0) ? v[1]-1 : total_vertex + v[1];
                vertex_indices[icount++] = (v[2] > 0) ? v[2]-1 : total_vertex + v[2];
            }
            else if (sscanf(line, "f %d %d %d", &index1, &index2, &index3) == 3) {
                vertex_indices[icount++] = (index1 > 0) ? index1-1 : total_vertex + index1;
                vertex_indices[icount++] = (index2 > 0) ? index2-1 : total_vertex + index2;
                vertex_indices[icount++] = (index3 > 0) ? index3-1 : total_vertex + index3;
            }
            // ignore other face formats for now
        }
    }

    fclose(obj);

    // If parsed fewer indices than estimated, shrink the indices array
    if (icount == 0) {
        printf("No faces parsed from OBJ: %s\n", filename);
        free(vertices);
        free(vertex_indices);
        return -1;
    }
    if (icount < indices_est) {
        int* tmp = (int*)realloc(vertex_indices, sizeof(int) * icount);
        if (tmp) vertex_indices = tmp;
    }

    // Compute bounds and normalize to [-0.5,0.5] scaled by largest dimension
    float minx, maxx, miny, maxy, minz, maxz;
    for (int k = 0; k < total_vertex; k++) {
        Vertex vt = vertices[k];
        if (k == 0) {
            minx = maxx = vt.position.x;
            miny = maxy = vt.position.y;
            minz = maxz = vt.position.z;
        } else {
            if (vt.position.x < minx) minx = vt.position.x;
            if (vt.position.x > maxx) maxx = vt.position.x;
            if (vt.position.y < miny) miny = vt.position.y;
            if (vt.position.y > maxy) maxy = vt.position.y;
            if (vt.position.z < minz) minz = vt.position.z;
            if (vt.position.z > maxz) maxz = vt.position.z;
        }
    }

    float width = maxx - minx;
    float height = maxy - miny;
    float depth = maxz - minz;
    float max_dimension = width;
    if (height > max_dimension) max_dimension = height;
    if (depth > max_dimension) max_dimension = depth;
    if (max_dimension == 0.0f) max_dimension = 1.0f;

    for (int k = 0; k < total_vertex; k++) {
        vertices[k].position.x = (vertices[k].position.x - minx - width / 2.0f) / max_dimension;
        vertices[k].position.y = (vertices[k].position.y - miny - height / 2.0f) / max_dimension;
        vertices[k].position.z = (vertices[k].position.z - minz - depth / 2.0f) / max_dimension;
        vertices[k].color = default_color; // Default color for all vertices (can be overridden by material info in future)
    }

    // Compute vertex normals by area-weighted sum of adjacent triangle faces
    Vector3* normalized_vertices = (Vector3*)calloc(total_vertex, sizeof(Vector3));
    if (!normalized_vertices) {
        printf("Out of memory allocating normals\n");
        free(vertices);
        free(vertex_indices);
        return -1;
    }

    for (int k = 0; k < icount; k += 3) {
        int i0 = vertex_indices[k];
        int i1 = vertex_indices[k+1];
        int i2 = vertex_indices[k+2];
        if (i0 < 0 || i0 >= total_vertex || i1 < 0 || i1 >= total_vertex || i2 < 0 || i2 >= total_vertex) continue;

        Vector3 v0 = vertices[i0].position;
        Vector3 v1 = vertices[i1].position;
        Vector3 v2 = vertices[i2].position;

        Vector3 area = vec_cross(vec_sub(v0, v1), vec_sub(v0, v2));
        normalized_vertices[i0] = vec_add(normalized_vertices[i0], area);
        normalized_vertices[i1] = vec_add(normalized_vertices[i1], area);
        normalized_vertices[i2] = vec_add(normalized_vertices[i2], area);
    }

    for (int k = 0; k < total_vertex; k++) {
        normalized_vertices[k] = vec_normalize(normalized_vertices[k]);
        // Flip normals if needed to ensure outward-facing (keep previous behavior)
        normalized_vertices[k].x = -normalized_vertices[k].x;
        normalized_vertices[k].y = -normalized_vertices[k].y;
        normalized_vertices[k].z = -normalized_vertices[k].z;
    }

    // Populate mesh struct
    mesh->Vertices = vertices;
    mesh->Vertex_normals = normalized_vertices;
    mesh->vertex_count = total_vertex;
    mesh->indices = vertex_indices;
    mesh->index_count = icount;
    
    // Allocate other per-mesh buffers
    mesh->rendered_triangles = (Triangle*)malloc(sizeof(Triangle) * (icount / 3));
    mesh->shaded_colors = (uint32_t*)malloc(sizeof(uint32_t) * total_vertex);
    mesh->transformed_normals = (Vector3*)malloc(sizeof(Vector3) * total_vertex);
    mesh->camera_vertices = (Vector3*)malloc(sizeof(Vector3) * total_vertex);
    mesh->projected_vertices = (Vertex*)malloc(sizeof(Vertex) * total_vertex);

    mesh -> rotation_angle = 0.0f;
    mesh -> translation = 0.0f;

    printf("Loaded OBJ: %s (vertices=%d, indices=%d)\n", filename, total_vertex, icount);
    return 0;
}
