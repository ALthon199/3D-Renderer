    #include "display.h"
    #include "renderer.h"
    #include "vector_math.h"
    #include "geometry.h"
    #include "mesh_utils.h"
    #include <math.h>
    #include <stdio.h>
    #include <omp.h>


    // The cross product of a given points and two other points.
    float edge_function(float px, float py, float x0, float y0, float x1, float y1) {
        return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
    }
    uint32_t get_shaded_color(uint32_t base_color, float intensity) {
        uint8_t r = (uint8_t)(((base_color >> 16) & 0xFF) * intensity);
        uint8_t g = (uint8_t)(((base_color >> 8) & 0xFF) * intensity);
        uint8_t b = (uint8_t)(((base_color >> 0) & 0xFF) * intensity);
        return (0xFF << 24) | (r << 16) | (g << 8) | (b << 0);
    }
    void put_pixel(Display* display, Vertex v) {
        // Snap float coordinates to integer pixel indices when coloring
        int ix = (int)(v.position.x);
        int iy = (int)(v.position.y);
        if (ix < 0 || ix >= WIDTH) {
            return;
        }
        if (iy < 0 || iy >= HEIGHT) {
            return;
        }

        
        int index = (iy * WIDTH) + ix;
        
        if (display->z_buffer[index] >  v.position.z) {
            display->z_buffer[index] = v.position.z;
            display->pixels[index] = v.color;
        }
    
    
        
    }
    static void draw_line_high(Display* display, Vertex v0, Vertex v1, uint32_t color) {
        if (v0.position.y > v1.position.y) {
            Vertex temp = v0;
            v0 = v1;
            v1 = temp;
        }

        int dx = (int)roundf(v1.position.x) - (int)roundf(v0.position.x);
        int dy = (int)roundf(v1.position.y) - (int)roundf(v0.position.y);
        int x = (int)roundf(v0.position.x);
        int xi = 1;
        if (dx < 0) {
            dx = -dx;
            xi = -1;
        }
            
        int D = 2 * dx - dy;

        for (int y = (int)roundf(v0.position.y); y < (int)roundf(v1.position.y) + 1; y++) {
            put_pixel(display, (Vertex){(Vector3){(float)x, (float)y, v0.position.z}, color});
            if (D > 0) {
                x = x + xi;
                D = D + 2 * (dx-dy);
            }
            else {
                D = D + 2 * dx;
            }
        }
    }
    static void draw_line_low(Display* display, Vertex v0, Vertex v1, uint32_t color) {
        if (v0.position.x > v1.position.x) {
            Vertex temp = v0;
            v0 = v1;
            v1 = temp;
        }

        int dx = (int)roundf(v1.position.x) - (int)roundf(v0.position.x);
        int dy = (int)roundf(v1.position.y) - (int)roundf(v0.position.y);
        int y = (int)roundf(v0.position.y);
        int yi = 1;
        
        if (dy < 0) {
            dy = -dy;
            yi = -1;
        }

        int D = 2 * dy - dx;
        for (int x = (int)roundf(v0.position.x); x < (int)roundf(v1.position.x) + 1; x++) {
            put_pixel(display, (Vertex){(Vector3){(float)x, (float)y, v0.position.z}, color});
            if (D > 0) {
                y = y + yi;
                D = D + 2 * (dy-dx);
            }
            else {
                D = D + 2 * dy;
            }
        }
    }
    void draw_line(Display* display, Vertex v0, Vertex v1) {
        // Bresenham's line algorithm works with integer coordinates
        // Color is that of v0 for simplicity
        int x0 = (int)roundf(v0.position.x);
        int y0 = (int)roundf(v0.position.y);
        int x1 = (int)roundf(v1.position.x);
        int y1 = (int)roundf(v1.position.y);

        if (abs(x0-x1) >= abs(y0-y1)) {
            draw_line_low(display, v0, v1, v0.color);
        }
        else {
            draw_line_high(display, v0, v1, v0.color);
        }
    }
    void draw_triangle(Display* display, Triangle* tri) {
        Vertex v0 = tri->v0; Vertex v1 = tri->v1; Vertex v2 = tri->v2;


        // 1. Bounding Box
        int minX = (int)fmaxf(0, floorf(fminf(v0.position.x, fminf(v1.position.x, v2.position.x))));
        int minY = (int)fmaxf(0, floorf(fminf(v0.position.y, fminf(v1.position.y, v2.position.y))));
        int maxX = (int)fminf(WIDTH - 1, ceilf(fmaxf(v0.position.x, fmaxf(v1.position.x, v2.position.x))));
        int maxY = (int)fminf(HEIGHT - 1, ceilf(fmaxf(v0.position.y, fmaxf(v1.position.y, v2.position.y))));

        float area = edge_function(v0.position.x, v0.position.y, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
        if (fabs(area) < 1e-6f) return;
        float inv_area = 1.0f / area;

        // 2. Precompute Gradients (Slopes)
        // How much does an attribute change if we move 1 pixel in X or Y?
        float w0_dx = v1.position.y - v0.position.y;
        float w1_dx = v2.position.y - v1.position.y;
        float w2_dx = v0.position.y - v2.position.y;
        float w0_dy = v0.position.x - v1.position.x;
        float w1_dy = v1.position.x - v2.position.x;
        float w2_dy = v2.position.x - v0.position.x;

        float dz_dx = (w0_dx * v0.position.z + w1_dx * v1.position.z + w2_dx * v2.position.z) * inv_area;
        float dz_dy = (w0_dy * v0.position.z + w1_dy * v1.position.z + w2_dy * v2.position.z) * inv_area;

        float r0 = (v0.color >> 16) & 0xFF, g0 = (v0.color >> 8) & 0xFF, b0 = v0.color & 0xFF;
        float r1 = (v1.color >> 16) & 0xFF, g1 = (v1.color >> 8) & 0xFF, b1 = v1.color & 0xFF;
        float r2 = (v2.color >> 16) & 0xFF, g2 = (v2.color >> 8) & 0xFF, b2 = v2.color & 0xFF;

        float dr_dx = (w0_dx * r0 + w1_dx * r1 + w2_dx * r2) * inv_area;
        float dr_dy = (w0_dy * r0 + w1_dy * r1 + w2_dy * r2) * inv_area;
        float dg_dx = (w0_dx * g0 + w1_dx * g1 + w2_dx * g2) * inv_area;
        float dg_dy = (w0_dy * g0 + w1_dy * g1 + w2_dy * g2) * inv_area;
        float db_dx = (w0_dx * b0 + w1_dx * b1 + w2_dx * b2) * inv_area;
        float db_dy = (w0_dy * b0 + w1_dy * b1 + w2_dy * b2) * inv_area;

        // 3. Initial values at the start of the bounding box (minX + 0.5, minY + 0.5)
        float w0_row = edge_function(minX + 0.5f, minY + 0.5f, v0.position.x, v0.position.y, v1.position.x, v1.position.y);
        float w1_row = edge_function(minX + 0.5f, minY + 0.5f, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
        float w2_row = edge_function(minX + 0.5f, minY + 0.5f, v2.position.x, v2.position.y, v0.position.x, v0.position.y);

        float z_row = (w0_row * v0.position.z + w1_row * v1.position.z + w2_row * v2.position.z) * inv_area;
        float r_row = (w0_row * r0 + w1_row * r1 + w2_row * r2) * inv_area;
        float g_row = (w0_row * g0 + w1_row * g1 + w2_row * g2) * inv_area;
        float b_row = (w0_row * b0 + w1_row * b1 + w2_row * b2) * inv_area;

        for (int j = minY; j <= maxY; j++) {
            float w0 = w0_row; float w1 = w1_row; float w2 = w2_row;
            float z = z_row; float r = r_row; float g = g_row; float b = b_row;

            for (int i = minX; i <= maxX; i++) {
                // Check if inside (inclusive of both CW and CCW for safety)
                if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                    int index = j * WIDTH + i;
                    if (z < display->z_buffer[index]) {
                        display->z_buffer[index] = z;
                        display->pixels[index] = (0xFF << 24) | ((uint8_t)r << 16) | ((uint8_t)g << 8) | (uint8_t)b;
                    }
                }
                // INCREMENT EVERYTHING
                w0 += w0_dx; w1 += w1_dx; w2 += w2_dx;
                z += dz_dx; r += dr_dx; g += dg_dx; b += db_dx;
            }
            w0_row += w0_dy; w1_row += w1_dy; w2_row += w2_dy;
            z_row += dz_dy; r_row += dr_dy; g_row += dg_dy; b_row += db_dy;
        }
    }
    void draw_triangle_bounded(Display* display, Triangle* tri, int minX, int minY, int maxX, int maxY) {
        
        Vertex v0 = tri->v0; Vertex v1 = tri->v1; Vertex v2 = tri->v2;
        
        
        float inv_area = tri->inv_area;
        if (inv_area > 100000.0f) inv_area = 100000.0f; 
        if (inv_area < -100000.0f) inv_area = -100000.0f;
        float w0_dx = tri->w0_dx;
        float w1_dx = tri->w1_dx;
        float w2_dx = tri->w2_dx;
        float w0_dy = tri->w0_dy;
        float w1_dy = tri->w1_dy;
        float w2_dy = tri->w2_dy;



        float dz_dx = tri->gradient.dz_dx;
        float dz_dy = tri->gradient.dz_dy;



        float dr_dx = tri->gradient.dr_dx;
        float dr_dy = tri->gradient.dr_dy;
        float dg_dx = tri->gradient.dg_dx;
        float dg_dy = tri->gradient.dg_dy;
        float db_dx = tri->gradient.db_dx;
        float db_dy = tri->gradient.db_dy;



        // 3. Initial values at the start of the bounding box (minX + 0.5, minY + 0.5)
        float w0_row = tri->w0_start + w0_dx * (minX - tri->minX) + w0_dy * (minY - tri->minY);
        float w1_row = tri->w1_start + w1_dx * (minX - tri->minX) + w1_dy * (minY - tri->minY);
        float w2_row = tri->w2_start + w2_dx * (minX - tri->minX) + w2_dy * (minY - tri->minY);

        
        float z_row = tri->z_start + dz_dx * (minX - tri->minX) + dz_dy * (minY - tri->minY);
        
        int r_row = tri->r_start + dr_dx * (minX - tri->minX) + dr_dy * (minY - tri->minY);
        int g_row = tri->g_start + dg_dx * (minX - tri->minX) + dg_dy * (minY - tri->minY);
        int b_row = tri->b_start + db_dx * (minX - tri->minX) + db_dy * (minY - tri->minY);

        for (int j = minY; j <= maxY; j++) {
            float w0 = w0_row; float w1 = w1_row; float w2 = w2_row;
            float z = z_row; int r = r_row; int g = g_row; int b = b_row;

            for (int i = minX; i <= maxX; i++) {
                

                // Check if inside (inclusive of both CW and CCW for safety)
                if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                    int index = j * WIDTH + i;
                    if (z < display ->z_buffer[index] - 0.0001f) {
                        display->z_buffer[index] = z;
                    

                        // Inside your inner pixel loop
                        int final_r_val = r >> FIXED_POINT_SHIFT & 0xFF;
                        int final_g_val = g >> FIXED_POINT_SHIFT & 0xFF;
                        int final_b_val = b >> FIXED_POINT_SHIFT & 0xFF;

                        // Clamp to 0-255 to prevent color "bleeding" or flickering
                        uint8_t final_r = (final_r_val > 255) ? 255 : (final_r_val < 0 ? 0 : final_r_val);
                        uint8_t final_g = (final_g_val > 255) ? 255 : (final_g_val < 0 ? 0 : final_g_val);
                        uint8_t final_b = (final_b_val > 255) ? 255 : (final_b_val < 0 ? 0 : final_b_val);

                        display->pixels[index] = (0xFF << 24) | (final_r << 16) | (final_g << 8) | final_b;
                    
                    }
                }
                // INCREMENT EVERYTHING
                w0 += w0_dx; w1 += w1_dx; w2 += w2_dx;
                z += dz_dx; r += dr_dx; g += dg_dx; b += db_dx;
            }
            w0_row += w0_dy; w1_row += w1_dy; w2_row += w2_dy;
            z_row += dz_dy; r_row += dr_dy; g_row += dg_dy; b_row += db_dy;
        }

    }
    void draw_mesh(Display* display, Mesh* mesh, Camera* camera) {
        int total_triangles = 0;

        Tile* tiles = display->tiles;
        int* fill_tracker = display->fill_tracker;
        int* global_pool = display->global_tiles;
        memset(fill_tracker, 0, sizeof(int) * ((WIDTH/16) * (HEIGHT/16)));
        memset(tiles, 0, sizeof(Tile) * ((WIDTH/16) * (HEIGHT/16)));
        

        float fov_factor = 600.0f;

        

        // Transform all vertices from world space to camera space, then project to screen space
    
        transform_mesh(display, mesh, camera);
    
        
        // Backface culling and triangle rasterization
        total_triangles = render_mesh(display, mesh, camera, tiles);
        prefix_sum_tiles(tiles, WIDTH/16 * HEIGHT/16);
        
        



        for (int i = 0; i < total_triangles; i++) {
            Triangle* tri = &(mesh->rendered_triangles[i]);
            Vertex v0 = tri->v0; Vertex v1 = tri->v1; Vertex v2 = tri->v2;
         
            for (int ty = tri->minY/16; ty <= tri->maxY/16; ty++) {
                for (int tx = tri->minX/16; tx <= tri->maxX/16; tx++) {
                    int index = ty * (WIDTH/16) + tx;
                
                    global_pool[tiles[index].offset + fill_tracker[index]] = i;
                    fill_tracker[index]++;
                    
                }
            }
        }

        int ty, tx, t;
        #pragma omp parallel for private(ty, tx, t) schedule(dynamic)
        for (ty = 0; ty < HEIGHT/16; ty++) {
            for (tx = 0; tx < WIDTH/16; tx++) {
                int index = ty * (WIDTH/16) + tx;
                for (t = 0; t < tiles[index].tri_count; t++) {
                    Triangle* tri = &mesh->rendered_triangles[global_pool[tiles[index].offset + t]];
                   
                    int tMinX = (int)tri->minX;
                    
                    int tMinY = (int)tri->minY;
                    int tMaxX = (int)ceilf(tri->maxX);
                    int tMaxY = (int)ceilf(tri->maxY);
                   
                    int rMinX = fmaxf(tx * 16, tMinX);
                    int rMinY = fmaxf(ty * 16, tMinY);
                    int rMaxX = fminf(tx * 16 + 15, tMaxX);
                    int rMaxY = fminf(ty * 16 + 15, tMaxY);

                    if (rMinX <= rMaxX && rMinY <= rMaxY) {
                        draw_triangle_bounded(display, tri, rMinX, rMinY, rMaxX, rMaxY);
                    }
                }
            }
        }
        
    }



    void free_mesh(Mesh* mesh) {
        free(mesh->Vertices);
        free(mesh->Vertex_normals);
        free(mesh->indices);
        free(mesh->camera_vertices);
        free(mesh->projected_vertices);
        free(mesh->transformed_normals);
        free(mesh->shaded_colors);

    }