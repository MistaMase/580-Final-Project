/*
 * Gz.h - include file for the cs580 rendering library
 */

/*
 * universal constants
 */
#define GZ_SUCCESS      0
#define GZ_FAILURE      1

/*
 * name list tokens
 */
#define GZ_NULL_TOKEN			0	/* triangle vert attributes */
#define GZ_POSITION				1
#define GZ_NORMAL				2
#define GZ_TEXTURE_INDEX		3


#define	GZ_AASHIFTX				44	/* antialiasing screen offset */
#define	GZ_AASHIFTY				45	/* antialiasing screen offset */

/* renderer-state default pixel color */
#define GZ_RGB_COLOR            99	

#define GZ_INTERPOLATE			95	/* define interpolation mode */

#define GZ_DIRECTIONAL_LIGHT	79	/* directional light */
#define GZ_AMBIENT_LIGHT		78	/* ambient light type */

#define GZ_AMBIENT_COEFFICIENT		1001	/* Ka material property */
#define GZ_DIFFUSE_COEFFICIENT		1002	/* Kd material property */
#define GZ_SPECULAR_COEFFICIENT		1003	/* Ks material property */
#define GZ_DISTRIBUTION_COEFFICIENT	1004	/* specular power of material */

#define	GZ_TEXTURE_MAP			1010	/* pointer to texture routine */
#define GZ_BUMP_MAP				1011	/* pointer to bump map routine */

/*
 * flags fields for value list attributes
 */

/* select interpolation mode of the shader (only one) */
#define GZ_FLAT			0	/* flat shading with GZ_RBG_COLOR */
#define	GZ_COLOR		1	/* interpolate vertex color */
#define	GZ_NORMALS		2	/* interpolate normals */

typedef int     GzToken;
typedef void    *GzPointer;
typedef float   GzColor[3];
typedef short   GzIntensity;	/* 0 - 4095 in lower 12-bits */
typedef float   GzCoord[3];
typedef float   GzTextureIndex[2];
typedef float   GzMatrix[4][4];
typedef int	GzDepth;		/* z is signed for clipping */

typedef	int	(*GzTexture)(float u, float v, GzColor color);	/* pointer to texture sampling method */
typedef int (*GzBump)(float u, float v, GzCoord given_normal, GzCoord& bump_map_normal);
/* u,v parameters [0,1] are defined tex_fun(float u, float v, GzColor color) */

/*
 * Gz camera definition
 */
#ifndef GZCAMERA
#define GZCAMERA
typedef struct  GzCamera
{
  GzMatrix			Xiw;  		/* xform from world to image space */
  GzMatrix			Xpi;  		/* perspective projection xform */
  GzCoord			position;  	/* position of image plane origin */
  GzCoord			lookat;         /* position of look-at-point */
  GzCoord			worldup;  /* world up-vector (almost screen up) */
  float				FOV;            /* horizontal field of view */
} GzCamera;
#endif

#ifndef GZLIGHT
#define GZLIGHT
typedef struct  GzLight
{
   GzCoord		direction; 	/* vector from surface to light */
   GzColor		color;		/* light color intensity */
} GzLight;
#endif

#ifndef GZINPUT
#define GZINPUT
typedef struct  GzInput
{
   GzCoord          rotation;       /* object rotation */
   GzCoord			translation;	/* object translation */
   GzCoord			scale;			/* object scaling */
   GzCamera			camera;			/* camera */
} GzInput;
#endif

#define RED     0        /* array indicies for color vector */
#define GREEN   1
#define BLUE    2

#define X       0        /* array indicies for position vector*/
#define Y       1
#define Z       2

#define U       0        /* array indicies for texture coords */
#define V       1


#ifndef GZ_PIXEL
typedef	struct {
  GzIntensity    red;	
  GzIntensity    green;
  GzIntensity    blue;
  GzIntensity    alpha;
  GzDepth	 z;
} GzPixel;
#define GZ_PIXEL
#endif;

#define	MAXXRES	1024	/* put some bounds on size in case of error */
#define	MAXYRES	1024

/* Vector and Matrix Algebra Helper Functions */
#ifndef MATRIX_OPS
#define MATRIX_OPS

inline void identity_matrix(GzMatrix& matrix) {
	matrix[0][0] = 1.0; 	matrix[0][1] = 0.0; 	matrix[0][2] = 0.0;	matrix[0][3] = 0.0;
	matrix[1][0] = 0.0; 	matrix[1][1] = 1.0; 	matrix[1][2] = 0.0;	matrix[1][3] = 0.0;
	matrix[2][0] = 0.0; 	matrix[2][1] = 0.0; 	matrix[2][2] = 1.0;	matrix[2][3] = 0.0;
	matrix[3][0] = 0.0; 	matrix[3][1] = 0.0; 	matrix[3][2] = 0.0;	matrix[3][3] = 1.0;
}

inline float dot_product(GzCoord a, GzCoord b) {
	return (a[X] * b[X] + a[Y] * b[Y] + a[Z] * b[Z]);
}

inline void dot_product(GzCoord a, GzCoord b, float& dst) {
	dst = (a[X] * b[X] + a[Y] * b[Y] + a[Z] * b[Z]);
}

inline void vector_scale(float scale, GzCoord src, GzCoord& dst) {
	dst[X] = src[X] * scale;
	dst[Y] = src[Y] * scale;
	dst[Z] = src[Z] * scale;
}

inline void vector_add(GzCoord a, GzCoord b, GzCoord& dst) {
	dst[X] = a[X] + b[X];
	dst[Y] = a[Y] + b[Y];
	dst[Z] = a[Z] + b[Z];
}

inline void vector_subtract(GzCoord a, GzCoord b, GzCoord& dst) {
	dst[X] = a[X] - b[X];
	dst[Y] = a[Y] - b[Y];
	dst[Z] = a[Z] - b[Z];
}

inline float normalization_factor(GzCoord& src) {
	return (float)sqrt(1.0 * src[X] * src[X] + 1.0 * src[Y] * src[Y] + 1.0 * src[Z] * src[Z]);
}

inline void normalize(GzCoord& src) {
	float norm = normalization_factor(src);
	src[X] = src[X] / norm;
	src[Y] = src[Y] / norm;
	src[Z] = src[Z] / norm;
}

inline void vector_cross_product(GzCoord a, GzCoord b, GzCoord& dst) {
	float c, d, e;
	c = (a[Y] * b[Z] - a[Z] * b[Y]);
	d = -1.0f * (a[X] * b[Z] - a[Z] * b[X]);
	e = (a[X] * b[Y] - a[Y] * b[X]);
	dst[X] = c;
	dst[Y] = d;
	dst[Z] = e;
}

inline void matrix_matrix_multiply(GzMatrix a, GzMatrix b, GzMatrix& dst) {
	GzMatrix tmp;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			tmp[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
		}
	}

	// Temporary matrix allows for in-place modifications
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			dst[i][j] = tmp[i][j];
		}
	}
}

inline void matrix_vector_multiply(GzCoord a, GzMatrix b, GzCoord& dst) {
	float w = b[3][0] * a[0] + b[3][1] * a[1] + b[3][2] * a[2] + b[3][3];
	float c = (b[0][0] * a[0] + b[0][1] * a[1] + b[0][2] * a[2] + b[0][3]) / w;
	float d = (b[1][0] * a[0] + b[1][1] * a[1] + b[1][2] * a[2] + b[1][3]) / w;
	float e = (b[2][0] * a[0] + b[2][1] * a[1] + b[2][2] * a[2] + b[2][3]) / w;
	dst[0] = c;
	dst[1] = d;
	dst[2] = e;
}

inline void matrix_vector_multiply_3d(GzCoord a, GzMatrix b, GzCoord& dst) {
	float c = (b[0][0] * a[0] + b[0][1] * a[1] + b[0][2] * a[2]);
	float d = (b[1][0] * a[0] + b[1][1] * a[1] + b[1][2] * a[2]);
	float e = (b[2][0] * a[0] + b[2][1] * a[1] + b[2][2] * a[2]);
	dst[0] = c;
	dst[1] = d;
	dst[2] = e;
}

inline void matrix_vector_multiply_offset(GzCoord a, GzMatrix b, GzCoord& dst, float offset_x, float offset_y) {
	float w = b[3][0] * a[0] + b[3][1] * a[1] + b[3][2] * a[2] + b[3][3];
	float c = (b[0][0] * a[0] + b[0][1] * a[1] + b[0][2] * a[2] + b[0][3]) / w;
	float d = (b[1][0] * a[0] + b[1][1] * a[1] + b[1][2] * a[2] + b[1][3]) / w;
	float e = (b[2][0] * a[0] + b[2][1] * a[1] + b[2][2] * a[2] + b[2][3]) / w;
	dst[0] = c + offset_x;
	dst[1] = d + offset_y;
	dst[2] = e;
}

#endif // !MATRIX_OPS