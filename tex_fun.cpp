/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

#define FRACTAL_DEPTH 8

#define TEXTURE_NAME "pebbles_texture.ppm"
#define BUMP_TEXTURE_NAME "pebbles_bump.ppm"

GzColor	*texture_image = NULL;
float* bump_image = NULL;
int texture_xs, texture_ys;
int bump_xs, bump_ys;
int texture_reset = 1;
int bump_reset = 1;

// Function prototype
int GzBilinearInterpolation(float u, float v, GzColor& interpolated_color, int image_size_x, int image_size_y, GzColor* image);
int computeBumpNormal(float u, float v, GzCoord given_normal, GzCoord& bump_map_normal, int image_size_x, int image_size_y, float* image);

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i;
  FILE			*fd;

  if (texture_reset) {          /* open and load texture file */
    fd = fopen (TEXTURE_NAME, "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &texture_xs, &texture_ys, &dummy);
    texture_image = (GzColor*)malloc(sizeof(GzColor)*(texture_xs+1)*(texture_ys+1));
    if (texture_image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < texture_xs*texture_ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      texture_image[i][RED] = (float)((int)pixel[RED]) * (1.0f / 255.0f);
      texture_image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0f / 255.0f);
      texture_image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0f / 255.0f);
      }

    texture_reset = 0;          /* init is done */
	fclose(fd);
  }

  // Invalid texture coordiantes - Set to closest possible valid value
  float new_u = u, new_v = v;
  if (new_u > 1.0) new_u = 1.0;
  if (new_u < 0.0) new_u = 0.0;
  if (new_v > 1.0) new_v = 1.0;
  if (new_v < 0.0) new_v = 0.0;

  // Bilinear interpolation
  int status = GZ_SUCCESS;
  GzColor interpolated_color;
  status |= GzBilinearInterpolation(new_u, new_v, interpolated_color, texture_xs, texture_ys, texture_image);
  color[RED] = interpolated_color[RED];
  color[GREEN] = interpolated_color[GREEN];
  color[BLUE] = interpolated_color[BLUE];

  return status;
}

/* Image texture function */
int bump_function(float u, float v, GzCoord given_normal, GzCoord& bump_normal)
{
    unsigned char pixel[1];
    unsigned char dummy;
    char foo[8];
    int i;
    FILE* fd;

    if (bump_reset) {          /* open and load texture file */
        fd = fopen(BUMP_TEXTURE_NAME, "rb");
        if (fd == NULL) {
            fprintf(stderr, "texture file not found\n");
            exit(-1);
        }
        fscanf(fd, "%s %d %d %c", foo, &bump_xs, &bump_ys, &dummy);
        bump_image = (float*)malloc(sizeof(float) * (bump_xs + 1) * (bump_ys + 1));
        if (bump_image == NULL) {
            fprintf(stderr, "malloc for texture image failed\n");
            exit(-1);
        }

        // TODO Double Check This
        for (i = 0; i < bump_xs * bump_ys; i++) {	/* create array of GzColor values */
            fread(pixel, sizeof(unsigned char), 1, fd);
            bump_image[i] = (float)((int)pixel[0]) * (1.0f / (pow(2, 8) - 1.0f));
        }

        bump_reset = 0;          /* init is done */
        fclose(fd);
    }

    // Invalid texture coordiantes - Set to closest possible valid value
    float new_u = u, new_v = v;
    if (new_u > 1.0) new_u = 1.0;
    if (new_u < 0.0) new_u = 0.0;
    if (new_v > 1.0) new_v = 1.0;
    if (new_v < 0.0) new_v = 0.0;

    // Bilinear interpolation
    int status = GZ_SUCCESS;
    GzCoord bump_map_normal;
    status |= computeBumpNormal(new_u, new_v, given_normal, bump_map_normal, bump_xs, bump_ys, bump_image);
    bump_normal[X] = bump_map_normal[X];
    bump_normal[Y] = bump_map_normal[Y];
    bump_normal[Z] = bump_map_normal[Z];

    return status;
}

/* Computes the surface normals of a grayscale texture image */
int computeBumpNormal(float u, float v, GzCoord given_normal, GzCoord& bump_map_normal, int image_size_x, int image_size_y, float* image) {
    // Status
    int status = GZ_SUCCESS;

    // Scale to size of texture
    float scaled_u = u * (image_size_x - 1);
    float scaled_v = v * (image_size_y - 1);

    // TODO Maybe make bump mapping work on the edges?
    // TODO Interpolate to pixel center and don't rely on the floor
    // Ensure u and v are within range for forward finite difference, otherwise just return 0 so it doesn't offset the normal
    int u_floor = (int)floor(scaled_u);
    int v_floor = (int)floor(scaled_v);
    if (u_floor + 1 > image_size_x - 1 || v_floor + 1 > image_size_y - 1 || u_floor < 0 || v_floor < 0) {
        bump_map_normal[X] = given_normal[X];
        bump_map_normal[Y] = given_normal[Y];
        bump_map_normal[Z] = given_normal[Z];
        return GZ_FAILURE;
    }

    // Displace the given normal
    //GzCoord 
    GzCoord gvn_normal;
    vector_scale(1.0, given_normal, gvn_normal);

    // Define the surface coordinate space
    GzCoord tangent = { 0.0, 1.0, 0.0 };
    GzCoord binormal;
    GzCoord projected_normal;
    vector_scale(dot_product(tangent, given_normal), given_normal, projected_normal);
    vector_subtract(tangent, projected_normal, tangent);
    normalize(tangent);
    vector_cross_product(tangent, given_normal, binormal);
    normalize(binormal);

    // Compute derivative in u and v directions from the bump map
    float surface_u = (image[v_floor * image_size_x + u_floor + 1] - image[v_floor * image_size_x + u_floor]) / 1.0f;
    float surface_v = (image[(v_floor + 1) * image_size_x + u_floor] - image[v_floor * image_size_x + u_floor]) / 1.0f;

    // Apply the bump offset
    GzCoord a, b, c, d, e;
    //vector_cross_product(tangent, given_normal, a);
    //vector_scale(surface_u, a, b);
    vector_scale(surface_u, binormal, b);
    vector_cross_product(binormal, given_normal, c);
    vector_scale(surface_v, c, d);
    vector_add(b, d, e);
    //normalize(vector)
    //vector_add(given_normal, e, bump_map_normal);
    normalize(bump_map_normal);

    return status;
}

/* Procedural Checkerboard Texture Function */
int procedural_checkerboard(float u, float v, GzColor color)
{
    // Invalid texture coordiantes - Set to closest possible valid value
    float new_u = u, new_v = v;
    if (new_u > 1.0) new_u = 1.0;
    if (new_u < 0.0) new_u = 0.0;
    if (new_v > 1.0) new_v = 1.0;
    if (new_v < 0.0) new_v = 0.0;

    // Checkerboard texture
    float NUM_U_SQUARES = 12;
    float NUM_V_SQUARES = 12;
    float u_square_division = 1.0f / NUM_U_SQUARES;
    float v_square_division = 1.0f / NUM_V_SQUARES;
    int u_square = (int)floor(new_u / u_square_division);
    int v_square = (int)floor(new_v / v_square_division);
    if ((u_square + v_square) % 2 == 0) {
        color[RED] = 0;
        color[GREEN] = 0;
        color[BLUE] = 0;
    }
    else {
        color[RED] = 1.0f;
        color[GREEN] = 1.0f;
        color[BLUE] = 1.0f;
    }

	return GZ_SUCCESS;
}

/* Recursive Function for generating fractal texture */
int fractal_helper(int depth, float u, float v, float xs, float xe, float ys, float ye) {
    if (!depth) return 1;

    float new_u = u, new_v = v;
    if (new_u > 1.0) new_u = 1.0;
    if (new_u < 0.0) new_u = 0.0;
    if (new_v > 1.0) new_v = 1.0;
    if (new_v < 0.0) new_v = 0.0;
    
    float x_step = (xe - xs) / 3;
    float x_mid_start = xs + x_step;
    float x_mid_end = xs + x_step * 2;
    float y_step = (ye - ys) / 3;
    float y_mid_start = ys + y_step;
    float y_mid_end = ys + y_step * 2;
    if (new_u > x_mid_start && new_u < x_mid_end && new_v > y_mid_start && new_v < y_mid_end) return 0;

    float new_xs, new_xe, new_ys, new_ye;
    if (new_u < x_mid_start) {
        new_xs = xs;
        new_xe = xs + x_step;
    }
    else if (new_u >= x_mid_start && new_u < x_mid_end) {
        new_xs = xs + x_step;
        new_xe = xs + x_step * 2;
    }
    else {
        new_xs = xs + x_step * 2;
        new_xe = xe;
    }
    if (new_v < y_mid_start) {
        new_ys = ys;
        new_ye = ys + y_step;
    }
    else if (new_v >= y_mid_start && new_v < y_mid_end) {
        new_ys = ys + y_step;
        new_ye = ys + y_step * 2;
    }
    else {
        new_ys = ys + y_step * 2;
        new_ye = ye;
    }
   
    return fractal_helper(depth - 1, new_u, new_v, new_xs, new_xe, new_ys, new_ye);
}

/* Procedular Fractal Texture Function */
int procedural_fractal(float u, float v, GzColor color) {
    // Evaluate the fractal
    float new_u = u, new_v = v;
    if (new_u > 1.0) new_u = 1.0;
    if (new_u < 0.0) new_u = 0.0;
    if (new_v > 1.0) new_v = 1.0;
    if (new_v < 0.0) new_v = 0.0;
    int value = fractal_helper(FRACTAL_DEPTH, new_u, new_v, 0, 1, 0, 1);

    color[RED] = (float)value;
    color[GREEN] = (float)value;
    color[BLUE] = (float)value;

    return GZ_SUCCESS;
}

/* Selects the procedural texture to use */
int ptex_fun(float u, float v, GzColor color) {
#if 1
    return procedural_fractal(u, v, color);
#else
    return procedural_checkerboard(u, v, color);
#endif
}

/* Free texture memory */
int GzFreeTexture()
{
	if (texture_image != NULL)
		free(texture_image);
    if (bump_image != NULL)
        free(bump_image);
	return GZ_SUCCESS;
}

int GzBilinearInterpolation(float u, float v, GzColor& interpolated_color, int image_size_x, int image_size_y, GzColor* image) {
    // Scale to size of texture
    float scaled_u = u * (image_size_x - 1);
    float scaled_v = v * (image_size_y - 1);

    // Take the ceiling for the "right" pixel and the floor for the "left" pixel
    int u_ceil = (int)ceil(scaled_u);
    int v_ceil = (int)ceil(scaled_v);
    int u_floor = (int)floor(scaled_u);
    int v_floor = (int)floor(scaled_v);

    // Bilinear interpolation based on floor and ceil of u and v
    float s_value = scaled_u - u_floor;
    float t_value = scaled_v - v_floor;
    interpolated_color[RED] =   s_value          * t_value           * image[image_size_x * v_ceil + u_ceil][RED] +
                                (1.0f - s_value) * t_value           * image[image_size_x * v_ceil + u_floor][RED] + 
                                s_value          * (1.0f - t_value)   * image[image_size_x * v_floor + u_ceil][RED] +
                                (1.0f - s_value) * (1.0f - t_value)   * image[image_size_x * v_floor + u_floor][RED];

    interpolated_color[GREEN] = s_value          * t_value           * image[image_size_x * v_ceil + u_ceil][GREEN] +
                                (1.0f - s_value) * t_value           * image[image_size_x * v_ceil + u_floor][GREEN] +
                                s_value          * (1.0f - t_value)   * image[image_size_x * v_floor + u_ceil][GREEN] +
                                (1.0f - s_value) * (1.0f - t_value)   * image[image_size_x * v_floor + u_floor][GREEN];

    interpolated_color[BLUE] =  s_value          * t_value           * image[image_size_x * v_ceil + u_ceil][BLUE] +
                                (1.0f - s_value) * t_value           * image[image_size_x * v_ceil + u_floor][BLUE] +
                                s_value          * (1.0f - t_value)   * image[image_size_x * v_floor + u_ceil][BLUE] +
                                (1.0f - s_value) * (1.0f - t_value)   * image[image_size_x * v_floor + u_floor][BLUE];

    return GZ_SUCCESS;
}