#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

//
// Array of triangles that should be rendered frame by frame
//
triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { 0, 0, 0 };

float fov_factor = 640;

//
// Global variables for execution status and game loop
//
bool is_running = false;
int previous_frame_time = 0;

//
// Setup function to initialise variables and game objects
//
void setup(void) {
    // Initialize render mode and triangle culling method
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;
    
    // Allocate the required memory in bytes for the color buffer
    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    // Creating a SDL texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    // Loads the cube values in the mesh data structure
    load_cube_mesh_data();
    // load_obj_file_data("./assets/cube.obj");
}

//
// Poll system events and handle keyboard input
//
void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT: 
            is_running = false;
            break;
        case SDL_KEYDOWN:
            // Handle key strokes
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    is_running = false;
                    break;
                case SDLK_1:
                    // Display the wireframe and a small red dot for each triangle vertex
                    printf("Mode: Show the wireframe and a small red dot for each triangle vertex.\n");
                    render_method = RENDER_WIRE_VERTEX;
                    break;
                case SDLK_2:
                    // Display only the wireframe lines
                    printf("Mode: Show only the wireframe lines.\n");
                    render_method = RENDER_WIRE;
                    break;
                case SDLK_3:
                    // Display filled triangles with a solid color
                    printf("Mode: Show filled triangles with a solid color.\n");
                    render_method = RENDER_FILL_TRIANGLE;
                    break;
                case SDLK_4:
                    // Display both filled triangles and wireframe lines
                    printf("Mode: Show both filled triangles and wireframe lines.\n");
                    render_method = RENDER_FILL_TRIANGLE_WIRE;
                    break;
                case SDLK_c:
                    // Toggle back-face culling
                    if (cull_method == CULL_BACKFACE) {
                        cull_method = CULL_NONE;
                    } else {
                        cull_method = CULL_BACKFACE;
                    }
                    printf("Mode: Back-face culling %s.\n", cull_method == CULL_BACKFACE ? "on" : "off");
                    
                    break;
            }
            break;
    }
}


//
// Function that recieves a 3D vector and return a projected 2D point
//
vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z,
    };
    return projected_point;
}

//
// Update function frame by frame with a fixed time step
//
void update(void) {
    // Wait some time until we reacg the target frame time in milliseconds
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    
    // Only delay execution if we are running to fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    
    previous_frame_time = SDL_GetTicks();

    // Initialise array of triangles to render
    triangles_to_render = NULL;
    
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;
    
    // Loop all triangle faces of our mesh
    int num_faces = array_length(mesh.faces);

    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec3_t transformed_vertices[3];

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Translate the vertex away from the camera in z
            transformed_vertex.z += 5;

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }
        
        // Backface culling test to see if the current face should be projected
        if (cull_method == CULL_BACKFACE) {
            // Check backface culling
            vec3_t vector_a = transformed_vertices[0]; /*   A   */
            vec3_t vector_b = transformed_vertices[1]; /*  / \  */
            vec3_t vector_c = transformed_vertices[2]; /* C---B */

            // Get the vector subtraction (B-A) and (C-A)
            vec3_t vector_ba = vec3_sub(vector_b, vector_a);
            vec3_t vector_ca = vec3_sub(vector_c, vector_a);
            // Normalize to normal vectors
            vec3_normalize(&vector_ba);
            vec3_normalize(&vector_ca);

            // Compute the face normal (using cross product to find perpendicular)
            // because the coordinate system is left handed the cross product will (AB cross CA)
            vec3_t normal = vec3_cross(vector_ba, vector_ca);

            // Normalize the face normal vector
            vec3_normalize(&normal);

            // Find the the vector between a point in the triangle and the camera origin
            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            // Calculate how aligned the camera ray is with the dot normal (using dot product)
            float dot_normal_camera = vec3_dot(normal, camera_ray);

            // Bypass triangle that are looking away from the camera
            if (dot_normal_camera < 0)
                continue;
        }
            
        vec2_t projected_points[3];

        // Loop all three vertices to perform the projection
        for (int j = 0; j < 3; j++) {
            // Project the current vertex
            projected_points[j] = project(transformed_vertices[j]);

            // Scale and translate the projected point to the middle of the screen
            projected_points[j].x += (window_width / 2);
            projected_points[j].y += (window_height / 2);

        }

        triangle_t projected_triangle = {
            .points = {
                { projected_points[0].x, projected_points[0].y },
                { projected_points[1].x, projected_points[1].y },
                { projected_points[2].x, projected_points[2].y }
            },
            .color = mesh_face.color
        };
   
        // Save the projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);
    }
}

//
// Render function to draw objects on the display 
//
void render(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_grid();

    // Loop all projected triangles and render them
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];
        
        if (render_method == RENDER_WIRE_VERTEX) {
        // // Draw vertex points
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
        }

        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            // Draw filled triangle
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                triangle.color
            );
        }

        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX 
            || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            // Draw unfilled triangle
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFFFFFFFF
            );
        }

    }
    
    // Clear the array of triangles to render
    array_free(triangles_to_render);

    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

//
// Free the memory that was dynamically allocated by the program
//
void free_resources(void) {
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

//
// Main function
//
int main(int argc, char* args[]) {
    
    // Create a SDL window
    is_running = initialize_window();

    setup();

    // Event loop
    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
