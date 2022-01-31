#include "display.h"
#include "swap.h"
#include "triangle.h"

//
// Draw a filled triangle with a flat bottom
//
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	// Find the two slopes (twp triangle legs)
	float inv_slope_1 = (float)(x1 - x0) / (y1 - y0); // inverted slope 1 (left)
	float inv_slope_2 = (float)(x2 - x0) / (y2 - y0); // inverted slope 2 (right)

	// Start x_start and x_end from the top vertex (x0,y0)
	float x_start = x0;
	float x_end = x0;

	// Loop all the scanlines from top to bottom
	for (int y = y0; y <= y2; y++) {
		draw_line(x_start, y, x_end, y, color);
		x_start += inv_slope_1;
		x_end += inv_slope_2;
	}

}

//
// Draw a filled triangle with a flat top
//
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	// Find the two slopes (two triangle legs) bottom up
	float inv_slope_1 = (float)(x2 - x0) / (y2 - y0); // inverted slope 1 (left)
	float inv_slope_2 = (float)(x2 - x1) / (y2 - y1); // inverted slope 2 (right)

	// Start x_start and x_end from the bottom vertex (x2,y2)
	float x_start = x2;
	float x_end = x2;

	// Loop all the scanlines from bottom to top
	for (int y = y2; y >= y0; y--) {
		draw_line(x_start, y, x_end, y, color);
		x_start -= inv_slope_1;
		x_end -= inv_slope_2;
	}
}

/*
// Draw a filled triangle using the flat-top/flat-bottom method. 
// Split the original triangle in two, half flat-bottom and half flat-top
//
//
//             (x0, y0)
//				 / \
//				/   \
//			   /     \
//			  /       \
//			 /         \
//		 (x1,y1)-------(Mx,My)
//			\_           \
//			   \_         \
//				  \_       \
//				     \_     \
//				        \    \
//				          \_  \
//				             \_\
//				                \
// 			                 (x2,y2)
*/
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	// We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
	}

	if (y1 > y2) {
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
	}

	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
	}

	if (y1 == y2) {
		// Draw flat-bottom triangle
		fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
	}
	else if (y0 == y1) {
		// Draw flat-top triangle
		fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
	}
	else {
		// Calculate the new vertex (Mx,My) using triangle similarity
		int My = y1;
		int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

		// Draw flat-bottom triangle
		fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

		// Draw flat-top triangle
		fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
	}
}

//
// Draw a triangle using three lines
//
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

/*
// Draw a textured triangle based on a texture array of colors.
// We split the original triangle in two, half flat-bottom and half flat-top.
//
//
//
//             	  v0
//				 / \
//				/   \
//			   /     \
//			  /       \
//			 /         \
//		   v1-----------\
//			\_           \
//			   \_         \
//				  \_       \
//				     \_     \
//				        \    \
//				          \_  \
//				             \_\
//				                \
// 			                    v2
*/
void draw_textured_triangle(
	int x0, int y0, float u0, float v0,
	int x1, int y1, float u1, float v1,
	int x2, int y2, float u2, float v2,
	uint32_t* texture
) {
	// We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}

	if (y1 > y2) {
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
		float_swap(&u1, &u2);
		float_swap(&v1, &v2);
	}

	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}

	//
	// Render the upper part of the triangle (flat-bottom)
	//
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with a custom color
                draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFFFF00FF : 0x00000000);
            }
        }
    }

	//
	// Render the bottom part of the triangle (flat-top)
	//
	inv_slope_1 = 0;
	inv_slope_2 = 0;

	if (y2 - y1 != 0) inv_slope_1 = (float) (x2 - x1) / abs(y2 - y1); // inverted slope 1 (left)
	if (y2 - y0 != 0) inv_slope_2 = (float) (x2 - x0) / abs(y2 - y0); // inverted slope 2 (right)

	if (y2 - y1 != 0) {
		for (int y = y1; y <= y2; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			if (x_end < x_start) 
				int_swap(&x_end, &x_start); // swap if x_start is to the right of x_end

			for (int x = x_start; x < x_end; x++) {
				// Draw pixel with the colour that comes form the texture
				draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFFFF00FF : 0x00000000);
			}
		}
	}
}
