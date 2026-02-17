#include "obj_loader.h"

#include <stdio.h>
    

int load_obj(const char* filename, Mesh* mesh) {
    
    FILE* obj = fopen(filename, "r");
    int total_vertex = 0;
    int indices = 0;

    
    char line[256];
    if (obj != NULL){
        
        while (fgets(line, sizeof(line), obj)){
           
            if (line[0] == 'v' && line[1] == ' '){
                total_vertex += 1;
                
            }
            else if (line[0] == 'f' && line[1] == ' '){
                indices += 3;
            }

        }
    }

    fseek(obj, 0, SEEK_SET);

    Vertex* vertices = (Vertex*)malloc(sizeof(Vertex) * total_vertex);
    int* vertex_indices = (int*)malloc(sizeof(int) * indices);

    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;


    int index1 = 0;
    int index2 = 0;
    int index3 = 0;

    int i = 0;
    int j = 0;
    if (obj != NULL){
        // Simple OBJ parser that only supports vertex positions and triangular faces
        while (fgets(line, sizeof(line), obj)){

            if (line[0] == 'v' && line[1] == ' '){
                sscanf(line, "v %f %f %f", &vx, &vy, &vz);
                vertices[i] = (Vertex){{vx, vy, vz}, 0xFFFF0000};
                i += 1;
               
            }
            else if (line[0] == 'f' && line[1] == ' '){
                sscanf(line, "f %d %d %d", &index1, &index2, &index3);
                vertex_indices[j] = index1-1;
                vertex_indices[j+1] = index2-1;
                vertex_indices[j+2] = index3-1;
                j += 3;
            }

        }
    }
    fclose(obj);


    // Normalize vertices to fit in unit cube centered at origin
    float minx, maxx, miny, maxy, minz, maxz;
    for (int k = 0; k < total_vertex; k++) {
        Vertex v = vertices[k];
        if (k == 0) {
            minx = maxx = v.position.x;
            miny = maxy = v.position.y;
            minz = maxz = v.position.z;
        }
        else {
            if ((float)v.position.x < minx) minx = (float)v.position.x;
            if ((float)v.position.x > maxx) maxx = (float)v.position.x;
            if ((float)v.position.y < miny) miny = (float)v.position.y;
            if ((float)v.position.y > maxy) maxy = (float)v.position.y;
            if ((float)v.position.z < minz) minz = (float)v.position.z;
            if ((float)v.position.z > maxz) maxz = (float)v.position.z;
        }
    }

    float width = maxx - minx;
    float height = maxy - miny;
    float depth = maxz - minz;

    float max_dimension = width;
    if (height > max_dimension) max_dimension = height;
    if (depth > max_dimension) max_dimension = depth;

    for (int k = 0; k < total_vertex; k++) {
        vertices[k].position.x = (vertices[k].position.x - minx - width / 2) / max_dimension;
        vertices[k].position.y = (vertices[k].position.y - miny - height / 2) / max_dimension;
        vertices[k].position.z = (vertices[k].position.z - minz - depth / 2) / max_dimension;
    }


    Vector3* normalized_vertices = (Vector3*)calloc( total_vertex, sizeof(Vector3));

    // Sum areas of adjacent triangles for each vertex normal
    for (int i = 0; i < indices; i+=3) {
        int i0 = vertex_indices[i];
        int i1 = vertex_indices[i+1];
        int i2 = vertex_indices[i+2];

        Vector3 v0 = vertices[i0].position;
        Vector3 v1 = vertices[i1].position;
        Vector3 v2 = vertices[i2].position;

        Vector3 area = vec_cross(vec_sub(v0, v1), vec_sub(v0, v2));

        normalized_vertices[i0] = vec_add(normalized_vertices[i0], area);
        normalized_vertices[i1] = vec_add(normalized_vertices[i1], area);
        normalized_vertices[i2] = vec_add(normalized_vertices[i2], area);
    }

    // Normalize the vertex normals
    for (int i = 0; i < total_vertex; i++){
        normalized_vertices[i] = vec_normalize(normalized_vertices[i]);
    }


    // Populate mesh struct
    mesh -> Vertices = vertices;
    mesh -> Vertex_normals = normalized_vertices;
    mesh -> vertex_count = total_vertex;
    mesh -> indices = vertex_indices;
    mesh -> index_count = indices;

    // Allocate memory for transformed vertices and normals that will be used during rendering
    mesh -> shaded_colors = (uint32_t*)malloc(sizeof(uint32_t) * total_vertex);
    mesh -> transformed_normals = (Vector3*)malloc(sizeof(Vector3) * total_vertex);
    mesh -> camera_vertices = (Vector3*)malloc(sizeof(Vector3) * total_vertex);
    mesh -> projected_vertices = (Vertex*)malloc(sizeof(Vertex) * total_vertex);

    printf("Normalized Bounds: %f to %f\n", vertices[0].position.x, vertices[total_vertex-1].position.x);
    printf("%d %d \n", total_vertex, indices);
    printf("Loading OBJ file: %s\n", filename);
    return 0; 
}

