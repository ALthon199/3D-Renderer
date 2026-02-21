
#include "mesh_utils.h"
#include <math.h>
#include <stdio.h>
#include <omp.h>


void prefix_sum_tiles(Tile* tiles, int total_tiles) {
    for (int i = 1; i < total_tiles; i++) {
        int count = tiles[i-1].tri_count;
        tiles[i].offset = tiles[i-1].offset + count;
    }
}

void transform_mesh(Display* display, Mesh* mesh, Camera* camera) {
    // This function is now empty because we moved the rendering code to draw_mesh in renderer.c
    // We still need to declare it here to avoid linker errors since it's called from main.c
    int i;
    #pragma omp parallel for private(i) schedule(dynamic)
    for (i = 0; i < mesh->vertex_count; i += 1) {
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
        
        // Blinn-Phong Lighting Model
        // 1. Directions (Assuming they are normalized)
        // L = Light Direction, V = View Direction (Vector from vertex to camera)
        // Directional light coming from above and in front of the object (positive Z towards camera)
        Vector3 light_dir = vec_normalize((Vector3){0.0f, -1.0f, 1.0f});
        Vector3 view_dir = vec_normalize(vec_sub((Vector3){0, 0, 0}, mesh->camera_vertices[i])); // Camera is at origin in camera space
        Vector3 L = light_dir;
        Vector3 H = vec_normalize(vec_add(L, view_dir));

        // 2. Diffuse (Lambertian)
        float dotNL = vec_dot(mesh->transformed_normals[i], L);
        float diffuse = fmaxf(dotNL, 0.0f);

        // 3. Specular (Blinn-Phong)
        float dotNH = vec_dot(mesh->transformed_normals[i], H);
        
        float spec_angle = fmaxf(dotNH, 0.0f);
        float s2 = spec_angle * spec_angle; // n=2
        float s4 = s2 * s2;                 // n=4
        float s8 = s4 * s4;                 // n=8
        float specular = s8 * s8;           // n=16
        

        // 4. Combine and clamp intensity to [0,1]
        float ambient = 0.1f;
        float intensity = ambient + (0.8f * diffuse) + (0.5f * specular);
        if (intensity < 0.0f) intensity = 0.0f;
        if (intensity > 1.0f) intensity = 1.0f;

        // 5. Final Color
        mesh->shaded_colors[i] = get_shaded_color(v0.color, intensity);

       
       
    }
}

int render_mesh(Display* display, Mesh* mesh, Camera* camera, Tile* tiles) {
   
    int total_triangles = 0;
    for (int i = 0; i < mesh->index_count; i+=3){

    
        Vector3 camerav0 = mesh->camera_vertices[mesh->indices[i]];
        Vector3 camerav1 = mesh->camera_vertices[mesh->indices[i+1]];
        Vector3 camerav2 = mesh->camera_vertices[mesh->indices[i+2]];

        

        Vector3 triangle_facing = vec_cross(vec_sub(camerav1, camerav0), vec_sub(camerav2, camerav0));
        // Vector from triangle to camera is (camera - triangle) = -camerav0 since camera is at origin in camera space
        Vector3 camera_facing = (Vector3){-camerav0.x, -camerav0.y, -camerav0.z};

        float dot = vec_dot(triangle_facing, camera_facing);
        // If dot <= 0, the triangle is facing away from the camera (backface) -> skip
        if (dot <= 0.0f) {
            continue;
        }
       

        Vertex v0 = mesh->projected_vertices[mesh->indices[i]];
        Vertex v1 = mesh->projected_vertices[mesh->indices[i+1]];
        Vertex v2 = mesh->projected_vertices[mesh->indices[i+2]];
        
        int minX = (int)fmaxf(0, floorf(fminf(v0.position.x, fminf(v1.position.x, v2.position.x))));
        int minY = (int)fmaxf(0, floorf(fminf(v0.position.y, fminf(v1.position.y, v2.position.y))));
        int maxX = (int)fminf(WIDTH - 1, ceilf(fmaxf(v0.position.x, fmaxf(v1.position.x, v2.position.x))));
        int maxY = (int)fminf(HEIGHT - 1, ceilf(fmaxf(v0.position.y, fmaxf(v1.position.y, v2.position.y))));

        if (maxX < 0 || maxY < 0 || minX >= WIDTH || minY >= HEIGHT) {
            continue; // Skip triangles that are completely off-screen
        }
        if (v0.position.z < 0.001f || v1.position.z < 0.001f || v2.position.z < 0.001f) {
           // Don't draw triangles that are behind the camera or too close to it
            continue;
        }
       
        v0.color = mesh->shaded_colors[mesh->indices[i]];
        v1.color = mesh->shaded_colors[mesh->indices[i+1]];
        v2.color = mesh->shaded_colors[mesh->indices[i+2]];

        

        float w0_dx = v1.position.y - v0.position.y;
        float w1_dx = v2.position.y - v1.position.y;
        float w2_dx = v0.position.y - v2.position.y;
        float w0_dy = v0.position.x - v1.position.x;
        float w1_dy = v1.position.x - v2.position.x;
        float w2_dy = v2.position.x - v0.position.x;
        float area = edge_function(v0.position.x, v0.position.y, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
        float inv_area = 1.0f / area;


        float dz_dx, dz_dy;
        dz_dx = (w0_dx * v0.position.z + w1_dx * v1.position.z + w2_dx * v2.position.z) * inv_area;
        dz_dy = (w0_dy * v0.position.z + w1_dy * v1.position.z + w2_dy * v2.position.z) * inv_area;

        uint32_t r0 = (v0.color >> 16) & 0xFF, g0 = (v0.color >> 8) & 0xFF, b0 = v0.color & 0xFF;
        uint32_t r1 = (v1.color >> 16) & 0xFF, g1 = (v1.color >> 8) & 0xFF, b1 = v1.color & 0xFF;
        uint32_t r2 = (v2.color >> 16) & 0xFF, g2 = (v2.color >> 8) & 0xFF, b2 = v2.color & 0xFF;

        int dr_dx = (int)((w0_dx * r0 + w1_dx * r1 + w2_dx * r2) * inv_area * (1 << FIXED_POINT_SHIFT));
        int dr_dy = (int)((w0_dy * r0 + w1_dy * r1 + w2_dy * r2) * inv_area * (1 << FIXED_POINT_SHIFT));
        int dg_dx = (int)((w0_dx * g0 + w1_dx * g1 + w2_dx * g2) * inv_area * (1 << FIXED_POINT_SHIFT));
        int dg_dy = (int)((w0_dy * g0 + w1_dy * g1 + w2_dy * g2) * inv_area * (1 << FIXED_POINT_SHIFT));
        int db_dx = (int)((w0_dx * b0 + w1_dx * b1 + w2_dx * b2) * inv_area * (1 << FIXED_POINT_SHIFT));
        int db_dy = (int)((w0_dy * b0 + w1_dy * b1 + w2_dy * b2) * inv_area * (1 << FIXED_POINT_SHIFT));
        Gradient gradient = {dz_dx, dz_dy, dr_dx, dr_dy, dg_dx, dg_dy, db_dx, db_dy};
        Vertex vertices[3] = {v0, v1, v2};
        Triangle tri = {vertices[0], vertices[1], vertices[2], gradient};
        mesh->rendered_triangles[total_triangles++] = tri;



        // Store Precomputed triangle data for tile-based rasterization optimization
    
        mesh->rendered_triangles[total_triangles-1].minX = minX;
        mesh->rendered_triangles[total_triangles-1].maxX = maxX;
        mesh->rendered_triangles[total_triangles-1].minY = minY;
        mesh->rendered_triangles[total_triangles-1].maxY = maxY;
        mesh->rendered_triangles[total_triangles-1].w0_dx = w0_dx;
        mesh->rendered_triangles[total_triangles-1].w0_dy = w0_dy;
        mesh->rendered_triangles[total_triangles-1].w1_dx = w1_dx;
        mesh->rendered_triangles[total_triangles-1].w1_dy = w1_dy;
        mesh->rendered_triangles[total_triangles-1].w2_dx = w2_dx;
        mesh->rendered_triangles[total_triangles-1].w2_dy = w2_dy;
        mesh->rendered_triangles[total_triangles-1].w0_start = edge_function(minX + 0.5f, minY + 0.5f, v0.position.x, v0.position.y, v1.position.x, v1.position.y);
        mesh->rendered_triangles[total_triangles-1].w1_start = edge_function(minX + 0.5f, minY + 0.5f, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
        mesh->rendered_triangles[total_triangles-1].w2_start = edge_function(minX + 0.5f, minY + 0.5f, v2.position.x, v2.position.y, v0.position.x, v0.position.y);
        mesh->rendered_triangles[total_triangles-1].z_start = (mesh->rendered_triangles[total_triangles-1].w0_start * v0.position.z + mesh->rendered_triangles[total_triangles-1].w1_start * v1.position.z + mesh->rendered_triangles[total_triangles-1].w2_start * v2.position.z) * inv_area;
        mesh->rendered_triangles[total_triangles-1].r_start = (int)((r0 << FIXED_POINT_SHIFT) + dr_dx * (minX + 0.5f - v0.position.x) + dr_dy * (minY + 0.5f - v0.position.y));
        mesh->rendered_triangles[total_triangles-1].g_start = (int)((g0 << FIXED_POINT_SHIFT) + dg_dx * (minX + 0.5f - v0.position.x) + dg_dy * (minY + 0.5f - v0.position.y));
        mesh->rendered_triangles[total_triangles-1].b_start = (int)((b0 << FIXED_POINT_SHIFT) + db_dx * (minX + 0.5f - v0.position.x) + db_dy * (minY + 0.5f - v0.position.y));
        mesh->rendered_triangles[total_triangles-1].inv_area = inv_area;
        
        for (int ty = minY/16; ty <= maxY/16; ty++) {
            for (int tx = minX/16; tx <= maxX/16; tx++) {
                int index = ty * (WIDTH/16) + tx;
             
                tiles[index].tri_count++;
                
                
            }
        }
    }
    return total_triangles;  
}