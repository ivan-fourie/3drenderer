#include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int window_width = 800;
int window_height = 600;

// Delcare a pointer to an array of uint32 elments
uint32_t* color_buffer = NULL;

// SDL Texture
SDL_Texture* color_buffer_texture = NULL;

bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    };

    // Use SDL to find out max. width and height for fullscreen
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    window_width = display_mode.w;
    window_height = display_mode.h;

    // Create a SDL Window
    window = SDL_CreateWindow(
            NULL, 
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width,
            window_height,
            SDL_WINDOW_BORDERLESS
    );
    
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");   
        return false;
    }
    
    // Create a SDL renderer
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    // Crashes on WSL :(
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

void draw_grid(void) {
    // Draw a background grid that fills the entire window.
    // Lines should be rendedered at every row/col multiple of 10.
    int grid_size = 10;
    uint32_t grid_color = 0xFF333333;

    for (int y = 0; y < window_height; y+=grid_size) {
        for (int x = 0; x < window_width; x+=grid_size) {
            color_buffer[(window_width * y) + x] = grid_color;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x < window_width && y < window_height) {
        color_buffer[(window_width * y) + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;
            color_buffer[(window_width * current_y) + current_x] = color;
        }
    }
}

void render_color_buffer(void) {
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int) (window_width * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color) {
    
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            color_buffer[(window_width * y) + x] = color;
        }
    }    
}

void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
