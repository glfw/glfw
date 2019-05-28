/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * Command line options:
 *    -info      print GL implementation information
 *    -exit      automatically exit after 30 seconds
 *
 *
 * Brian Paul
 *
 *
 * Marcus Geelnard:
 *   - Conversion to GLFW
 *   - Time based rendering (frame rate independent)
 *   - Slightly modified camera that should work better for stereo viewing
 *
 *
 * Camilla Löwy:
 *   - Removed FPS counter (this is not a benchmark)
 *   - Added a few comments
 *   - Enabled vsync
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

/*
 * mesh.h
 *
 *   Structures used to transform a list of triangles into a triangle strip.
 *
 *   Etienne Belanger
 */
typedef struct
{
  float e[3];
}
vect3x32;

struct vertex
{
  vect3x32 pos;
  vect3x32 norm;
};

struct trilist
{
  struct vertex * vertices;
  unsigned short vcount;
 
  unsigned short * indices;
  unsigned short icount;
};
/*
 * end of mesh.h
 */

#define DEG2RAD 0.017453292519943295769236907684886f
static const float deg2rad = DEG2RAD;

static const float halfpi = 1.5707963267948966192313216916398f;
static const float pi = 3.1415926535897932384626433832795f;
static const float twopi = 6.283185307179586476925286766559f;

static GLfloat view_rotx = 20.0f * DEG2RAD, view_roty = 30.0f * DEG2RAD, view_rotz = 0.0f;
static GLfloat angle = 0.0f;

/**
 
  Changes for ES 2.0 Compliance:
 
    * Store one OpenGL program number.
 
    * Use two buffers to store vertex data and index data.
 
    * Three trilist structures will be used to gather geometry so that it can be
      copied to a vertex buffer object.
 
    * Store the locations of all the program uniforms.
 
    * The fixed function transformation pipeline is not supported as per ES 2.0 spec para 2.11.
      Matrices are required to perform the same transformations without the OpenGL matrix stack.
 
 **/
 
/* there will be only one program for this demo */
static GLuint program;

/* two buffers: one for the vertex data and one for the indices */
static GLuint buffers[2];

/* three triangle lists for each spinning gear */
static struct trilist gear1, gear2, gear3;

/* location of the ModelViewProjectionMatrix uniform used in the vertex shader */
static GLuint mvp_matrix_loc;

/* location of the NormalMatrix uniform used in the vertex shader */
static GLuint normal_matrix_loc;

/* location of the LightPosition uniform used in the vertex shader */
static GLuint light_pos_loc;

/* location of the LightModelProduct uniform used in the vertex shader */
static GLuint lm_prod_loc;

/* location of the AmbientDiffuse uniform used in the vertex shader */
static GLuint ambient_diffuse_loc;

/* used for assignments in translation and rotation functions */
static const GLfloat Identity[16] =
{
  1.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 1.0f
};

/* updated for each gear */
static GLfloat ModelViewProjectionMatrix[16];

/* updated when the window is reshaped */
static GLfloat ProjectionMatrix[16];

/* updated everytime the view matrix changes (keystrokes) */
static GLfloat ViewMatrix[16];

/* OpenGL can't extract a 3x3 matrix out of a 4x4 matrix with Uniform[] calls.
   We need a separate matrix that will store the upper left 3x3 portion of the ModelView matrix. */
static GLfloat NormalMatrix[9];

/**
 
  Simply copies the vertex position and normal coefficients to the array
  element pointed to by the count pointer. The function also increments the
  value pointed to by count so that add_vertex can be called sequentially
  to add vertices.
 
 **/
 
static void add_vertex(struct vertex * vertices, unsigned short * count, const struct vertex * v)
{
  vertices[*count].pos.e[0] = v->pos.e[0];
  vertices[*count].pos.e[1] = v->pos.e[1];
  vertices[*count].pos.e[2] = v->pos.e[2];
  vertices[*count].norm.e[0] = v->norm.e[0];
  vertices[*count].norm.e[1] = v->norm.e[1];
  vertices[*count].norm.e[2] = v->norm.e[2];
  ++(*count);
}

/**
 
  Draw a gear wheel.  You'll probably want to call this function when
  building a display list since we do a lot of trig here.
  
  Input:  inner_radius - radius of hole at center
          outer_radius - radius at center of teeth
          width - width of gear
          teeth - number of teeth
          tooth_depth - depth of tooth
 
 **/
 
/**
 
  Changes for ES 2.0 compliance:
 
    * ShadeModel is not supported as per ES 2.0 spec para 2.14.
      Flat shading can be implemented by duplicating shared vertices so that they each
      have a normal that corresponds to the face normal.
 
    * Display lists are not supported as per ES 2.0 spec paras 2.4 and 5.4.
      Immediate mode is not supported as per ES 2.0 spec para 2.6.
      We cannot use the notion of current vertex as per ES 2.0 spec para 2.7.
      Instead, the vertex positions and normals--one for each position--will be accumulated
      in a buffer to be used as a vertex array. The vertex buffer data will also
      be stored in a vertex buffer object for better performance.
 
    * Indexed drawing is used to improve performance by reducing the number of
      vertices transformed from 2160 to 1040, which is less than half.
      Indexed drawing is supported as per ES 2.0 spec para 2.8.
 
    * GL_QUADS and GL_QUAD_STRIP are not supported as per ES 2.0 spec para 2.6.
      The primitives were replaced by triangles.
 
    * The indices were manually gathered to avoid expensive searches for matching
      vertices.
 
 **/
 
static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth, GLushort index_offset, struct trilist * triangles)
{
    GLint i;
    GLfloat r0, r1, r2;
    GLfloat angle, da, inc;
    GLfloat u1, v1, u2, v2, invlen;
    GLfloat cos_angle, sin_angle;
    GLfloat cos_angle_next, sin_angle_next;
    GLfloat cos_angle_1da, sin_angle_1da;
    GLfloat cos_angle_2da, sin_angle_2da;
    GLfloat cos_angle_3da, sin_angle_3da;
    GLfloat half_width;
    struct vertex v;
    unsigned short start;
  
    r0 = inner_radius;
    r1 = outer_radius - tooth_depth * 0.5f;
    r2 = outer_radius + tooth_depth * 0.5f;
  
    da = halfpi / teeth;
    inc = twopi / teeth;
    half_width = width * 0.5f;
  
    /* we know in advance that there will be 30 vertices per tooth + 4 */
    triangles->vertices = malloc((28 * teeth + 4) * sizeof(struct vertex));
    triangles->vcount = 0;
  
    /* we know in advance that there are 20 triangles per tooth + 2 */
    triangles->indices = malloc((60 + 6) * teeth * sizeof(unsigned short));
    triangles->icount = 0;
  
    /* all flat shading loops were collapsed into one */
    for (i = 0, angle = 0.0f; i < teeth; i++, angle += inc) {
        /* pre-compute these because they will be reused a lot */
        cos_angle = cos(angle);
        sin_angle = sin(angle);
        cos_angle_next = cos(angle + inc);
        sin_angle_next = sin(angle + inc);
        cos_angle_1da = cos(angle + da);
        sin_angle_1da = sin(angle + da);
        cos_angle_2da = cos(angle + 2.0f * da);
        sin_angle_2da = sin(angle + 2.0f * da);
        cos_angle_3da = cos(angle + 3.0f * da);
        sin_angle_3da = sin(angle + 3.0f * da);
  
        /* u1 and v1 are not texture coordinates but normal components */
        u1 = r2 * cos_angle_1da - r1 * cos_angle;
        v1 = r2 * sin_angle_1da - r1 * sin_angle;
        invlen = 1.0f / sqrt(u1 * u1 + v1 * v1);
        u1 *= invlen;
        v1 *= invlen;
  
        /* u2 and v2 are not texture coordinates but normal components */
        u2 = r1 * cos_angle_3da - r2 * cos_angle_2da;
        v2 = r1 * sin_angle_3da - r2 * sin_angle_2da;
        invlen = 1.0f / sqrt(u2 * u2 + v2 * v2);
        u2 *= invlen;
        v2 *= invlen;
  
        /*******************************************************/
        /* triangles from inner radius to outer radius (right) */
  
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 1;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 2;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 2;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 0 : triangles->vcount + 26);
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 2;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 1 : triangles->vcount + 27);
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 0 : triangles->vcount + 26);
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 1;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 3;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 4;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 1;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 4;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 2;
  
        /******************************************************/
        /* triangles from inner radius to outer radius (left) */
  
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 5;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 7;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 6;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 5;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 5 : triangles->vcount + 31);
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 7;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 7;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 5 : triangles->vcount + 31);
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 6 : triangles->vcount + 32);
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 6;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 9;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 8;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 6;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 7;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 9;
  
        /******************************/
        /* outside faces of the wheel */
  
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 10;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 11;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 12;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 12;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 11;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 13;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 14;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 15;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 16;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 16;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 15;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 17;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 18;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 19;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 20;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 20;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 19;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 21;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 22;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 23;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 24 : triangles->vcount + 50);
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 24 : triangles->vcount + 50);
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 23;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth-1 ? 25 : triangles->vcount + 51);
  
        /************************************/
        /* a list of vertices for this loop */
  
        /* vcount + 0 */
        v.pos.e[0] = r0 * cos_angle;
        v.pos.e[1] = r0 * sin_angle;
        v.pos.e[2] = half_width;
        v.norm.e[0] = 0.0f;
        v.norm.e[1] = 0.0f;
        v.norm.e[2] = 1.0f;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 1 */
        v.pos.e[0] = r1 * cos_angle;
        v.pos.e[1] = r1 * sin_angle;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 2 */
        v.pos.e[0] = r1 * cos_angle_3da;
        v.pos.e[1] = r1 * sin_angle_3da;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 3 */
        v.pos.e[0] = r2 * cos_angle_1da;
        v.pos.e[1] = r2 * sin_angle_1da;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 4 */
        v.pos.e[0] = r2 * cos_angle_2da;
        v.pos.e[1] = r2 * sin_angle_2da;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 5 */
        v.pos.e[0] = r0 * cos_angle;
        v.pos.e[1] = r0 * sin_angle;
        v.pos.e[2] = -half_width;
        v.norm.e[2] = -1.0f;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 6 */
        v.pos.e[0] = r1 * cos_angle;
        v.pos.e[1] = r1 * sin_angle;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 7 */
        v.pos.e[0] = r1 * cos_angle_3da;
        v.pos.e[1] = r1 * sin_angle_3da;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 8 */
        v.pos.e[0] = r2 * cos_angle_1da;
        v.pos.e[1] = r2 * sin_angle_1da;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 9 */
        v.pos.e[0] = r2 * cos_angle_2da;
        v.pos.e[1] = r2 * sin_angle_2da;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 10 */
        v.pos.e[0] = r1 * cos_angle;
        v.pos.e[1] = r1 * sin_angle;
        v.pos.e[2] = half_width;
        v.norm.e[0] = v1;
        v.norm.e[1] = -u1;
        v.norm.e[2] = 0.0f;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 11 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 12 */
        v.pos.e[0] = r2 * cos_angle_1da;
        v.pos.e[1] = r2 * sin_angle_1da;
        v.pos.e[2] = half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 13 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 14 */
        v.pos.e[0] = r2 * cos_angle_1da;
        v.pos.e[1] = r2 * sin_angle_1da;
        v.pos.e[2] = half_width;
        v.norm.e[0] = cos_angle;
        v.norm.e[1] = sin_angle;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 15 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 16 */
        v.pos.e[0] = r2 * cos_angle_2da;
        v.pos.e[1] = r2 * sin_angle_2da;
        v.pos.e[2] = half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 17 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 18 */
        v.pos.e[0] = r2 * cos_angle_2da;
        v.pos.e[1] = r2 * sin_angle_2da;
        v.pos.e[2] = half_width;
        v.norm.e[0] = v2;
        v.norm.e[1] = -u2;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 19 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 20 */
        v.pos.e[0] = r1 * cos_angle_3da;
        v.pos.e[1] = r1 * sin_angle_3da;
        v.pos.e[2] = half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 21 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 22 */
        v.pos.e[0] = r1 * cos_angle_3da;
        v.pos.e[1] = r1 * sin_angle_3da;
        v.pos.e[2] = half_width;
        v.norm.e[0] = cos_angle;
        v.norm.e[1] = sin_angle;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 23 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 24 */
        v.pos.e[0] = r1 * cos_angle;
        v.pos.e[1] = r1 * sin_angle;
        v.pos.e[2] = half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 25 */
        v.pos.e[2] = -half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
    }
  
    start = triangles->vcount;
  
    /* inside radius cylinder */
    for (i = 0, angle = 0.0f; i <= teeth; i++, angle += inc) {
        cos_angle = cos(angle);
        sin_angle = sin(angle);
        cos_angle_next = cos(angle + inc);
        sin_angle_next = sin(angle + inc);
  
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount;
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 1;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth ? start : triangles->vcount + 2);
        triangles->indices[triangles->icount++] = index_offset + triangles->vcount + 1;
        triangles->indices[triangles->icount++] = index_offset + (i == teeth ? start + 1 : triangles->vcount + 3);
        triangles->indices[triangles->icount++] = index_offset + (i == teeth ? start : triangles->vcount + 2);
  
        /************************************/
        /* a list of vertices for this loop */
  
        /* vcount + 0 */
        v.pos.e[0] = r0 * cos_angle;
        v.pos.e[1] = r0 * sin_angle;
        v.pos.e[2] = -half_width;
        v.norm.e[0] = -cos_angle;
        v.norm.e[1] = -sin_angle;
        v.norm.e[2] = 0.0f;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
  
        /* vcount + 1 */
        v.pos.e[2] = half_width;
        add_vertex(triangles->vertices, &triangles->vcount, &v);
    }
}

/**
 
  This function creates an OpenGL program that has one vertex shader and one
  fragment shader sourced from the gears.vs and gears.fs respectively. The function
  also fetches the uniform locations in some global variables so that their values
  can be changed later on.
 
 **/
 
static void load_program(void)
{
    const char *vtx_shdr_src =
        "// gears.vs\n"
        "//\n"
        "// Emulates a fixed-function pipeline with:\n"
        "//  GL_ALPHA_TEST is disabled by default\n"
        "//  GL_BLEND is disabled by default\n"
        "//  GL_CLIP_PLANEi is disabled\n"
        "//  GL_LIGHTING and GL_LIGHT0 enabled\n"
        "//  GL_FOG disabled\n"
        "//  GL_TEXTURE_xx disabled\n"
        "//  GL_TEXTURE_GEN_x disabled\n"
        "//\n"
        "//  GL_LIGHT_MODEL_AMBIENT is never set -> default value is (0.2, 0.2, 0.2, 1.0)\n"
        "//\n"
        "//  GL_LIGHT0 position is (5, 5, 10, 0)\n"
        "//  GL_LIGHT0 ambient is never set -> default value is (0, 0, 0, 1)\n"
        "//  GL_LIGHT0 diffuse is never set -> default value is (1, 1, 1, 1)\n"
        "//  GL_LIGHT0 specular is never set -> default value is (1, 1, 1, 1)\n"
        "//\n"
        "//  GL_AMBIENT and GL_DIFFUSE are (red/green/blue)\n"
        "//  GL_SPECULAR is never set -> default value is (0, 0, 0, 1)\n"
        "//  GL_EMISSION is never set -> default value is (0, 0, 0, 1)\n"
        "//  GL_SHININESS is never set -> default value is 0\n"
        "//\n"
        "//  ES 2.0 only supports generic vertex attributes as per spec para 2.6\n"
        "//\n"
        "//  Combining material with light settings gives us directional diffuse with\n"
        "//  ambient from light model.\n"
        "//\n"
        "//  Since alpha test and blend are both disabled, there is no need to keep track\n"
        "//  of the alpha component when shading. This saves the interpolation of one\n"
        "//  float between vertices and brings the performance back to the level of the\n"
        "//  'fixed function pipeline' when the window is maximized.\n"
        "//\n"
        "//  App is responsible for normalizing LightPosition.\n"
        "\n"
        "// should be set to gl_ProjectionMatrix * gl_ModelViewMatrix\n"
        "uniform mat4 ModelViewProjectionMatrix;\n"
        "\n"
        "// should be same as mat3(gl_ModelViewMatrix) because gl_ModelViewMatrix is orthogonal\n"
        "uniform mat3 NormalMatrix;\n"
        "\n"
        "// should be set to 'normalize(5.0, 5.0, 10.0)'\n"
        "uniform vec3 LightPosition;\n"
        "\n"
        "// should be set to AmbientDiffuse * vec4(0.2, 0.2, 0.2, 1.0)\n"
        "uniform vec3 LightModelProduct;\n"
        "\n"
        "// will be set to red/green/blue\n"
        "uniform vec3 AmbientDiffuse;\n"
        "\n"
        "// user defined attribute to replace gl_Vertex\n"
        "attribute vec4 Vertex;\n"
        "\n"
        "// user defined attribute to replace gl_Normal\n"
        "attribute vec3 Normal;\n"
        "\n"
        "// interpolate per-vertex lighting\n"
        "varying vec3 Color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "  // transform the vertex and normal in eye space\n"
        "  gl_Position = ModelViewProjectionMatrix * Vertex;\n"
        "  vec3 NormEye = normalize(NormalMatrix * Normal);\n"
        "\n"
        "  // do one directional light (diffuse only); no need to clamp for this example\n"
        "  Color = LightModelProduct + AmbientDiffuse * max(0.0, dot(NormEye, LightPosition));\n"
        "}\n";

    const char *frg_shdr_src =
        "// gears.fs\n"
        "//\n"
        "// Emulates a fixed function pipeline with:\n"
        "//  GL_ALPHA_TEST is disabled by default\n"
        "//  GL_BLEND is disabled by default\n"
        "//  GL_FOG disabled\n"
        "//  GL_TEXTURE_xx disabled\n"
        "//\n"
        "//  Since alpha test and blend are both disabled, there is no need to keep track\n"
        "//  of the alpha component in the fragment shader.\n"
        "\n"
        "// interpolated per-vertex lighting\n"
        "varying lowp vec3 Color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "  // this is the simplest fragment shader\n"
        "  gl_FragColor.rgb = Color;\n"
        "}\n";
  
    GLuint vertex_shader, fragment_shader;
  
    program = glCreateProgram();
  
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glAttachShader(program, vertex_shader);
    glShaderSource(vertex_shader, 1, (const char**)&vtx_shdr_src, NULL);
    glCompileShader(vertex_shader);
  
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glAttachShader(program, fragment_shader);
    glShaderSource(fragment_shader, 1, (const char**)&frg_shdr_src, NULL);
    glCompileShader(fragment_shader);
  
    glLinkProgram(program);
    glUseProgram(program);
  
    mvp_matrix_loc = glGetUniformLocation(program, "ModelViewProjectionMatrix");
    normal_matrix_loc = glGetUniformLocation(program, "NormalMatrix");
    light_pos_loc = glGetUniformLocation(program, "LightPosition");
    lm_prod_loc = glGetUniformLocation(program, "LightModelProduct");
    ambient_diffuse_loc = glGetUniformLocation(program, "AmbientDiffuse");
}

/**
 
  Creates a perspective matrix as per OpenGL definition of glFrustum.
 
 **/
 
static void frustum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat znear, GLfloat zfar)
{
    ProjectionMatrix[0] = 2.0f * znear / (right - left);
    ProjectionMatrix[1] = 0.0f;
    ProjectionMatrix[2] = 0.0f;
    ProjectionMatrix[3] = 0.0f;
    ProjectionMatrix[4] = 0.0f;
    ProjectionMatrix[5] = 2.0f * znear / (top - bottom);
    ProjectionMatrix[6] = 0.0f;
    ProjectionMatrix[7] = 0.0f;
    ProjectionMatrix[8] = (right + left) / (right - left);
    ProjectionMatrix[9] = (top + bottom) / (top - bottom);
    ProjectionMatrix[10] = -(zfar + znear) / (zfar - znear);
    ProjectionMatrix[11] = -1.0f;
    ProjectionMatrix[12] = 0.0f;
    ProjectionMatrix[13] = 0.0f;
    ProjectionMatrix[14] = -2.0f * zfar * znear / (zfar - znear);
    ProjectionMatrix[15] = 0.0f;
}

/**
 
  Copies the upper 3x3 matrix of 4x4 matrix B into 3x3 matrix A.
  The result is also the return value to allow cascading matrix operations.
 
 **/
 
static GLfloat *matrix_copy_3x3(GLfloat *A, const GLfloat *B)
{
    A[0] = B[0];
    A[1] = B[1];
    A[2] = B[2];
    A[3] = B[4];
    A[4] = B[5];
    A[5] = B[6];
    A[6] = B[8];
    A[7] = B[9];
    A[8] = B[10];
    return A;
}

/**
 
  Stores the multiplication of matrix A and B into matrix B.
  The result is also the return value to allow cascading matrix operations.
 
 **/
 
static GLfloat *matrix_rmultiply(const GLfloat *A, GLfloat *B)
{
    GLfloat T[3];
  
    T[0] = B[0];
    T[1] = B[1];
    T[2] = B[2];
  
    B[0] = A[0] * B[0] + A[4] * B[1] + A[8] * B[2] + A[12] * B[3];
    B[1] = A[1] * T[0] + A[5] * B[1] + A[9] * B[2] + A[13] * B[3];
    B[2] = A[2] * T[0] + A[6] * T[1] + A[10] * B[2] + A[14] * B[3];
    B[3] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[3];
  
    T[0] = B[4];
    T[1] = B[5];
    T[2] = B[6];
  
    B[4] = A[0] * B[4] + A[4] * B[5] + A[8] * B[6] + A[12] * B[7];
    B[5] = A[1] * T[0] + A[5] * B[5] + A[9] * B[6] + A[13] * B[7];
    B[6] = A[2] * T[0] + A[6] * T[1] + A[10] * B[6] + A[14] * B[7];
    B[7] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[7];
  
    T[0] = B[8];
    T[1] = B[9];
    T[2] = B[10];
  
    B[8] = A[0] * B[8] + A[4] * B[9] + A[8] * B[10] + A[12] * B[11];
    B[9] = A[1] * T[0] + A[5] * B[9] + A[9] * B[10] + A[13] * B[11];
    B[10] = A[2] * T[0] + A[6] * T[1] + A[10] * B[10] + A[14] * B[11];
    B[11] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[11];
  
    T[0] = B[12];
    T[1] = B[13];
    T[2] = B[14];
  
    B[12] = A[0] * B[12] + A[4] * B[13] + A[8] * B[14] + A[12] * B[15];
    B[13] = A[1] * T[0] + A[5] * B[13] + A[9] * B[14] + A[13] * B[15];
    B[14] = A[2] * T[0] + A[6] * T[1] + A[10] * B[14] + A[14] * B[15];
    B[15] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[15];
  
    return B;
}

/**
 
  Equivalent to matrix_multiply(A, rotation_x(tmp, angle)).
  This is faster because two rows remain unchanged by the rotation in matrix A.
 
 **/
 
static GLfloat *rotate_x(GLfloat *A, float angle)
{
    GLfloat cosa = cos(angle);
    GLfloat sina = sin(angle);
    GLfloat T[4];
  
    T[0] = A[4];
    T[1] = A[5];
    T[2] = A[6];
    T[3] = A[7];
  
    A[4] = T[0] * cosa + A[8] * sina;
    A[5] = T[1] * cosa + A[9] * sina;
    A[6] = T[2] * cosa + A[10] * sina;
    A[7] = T[3] * cosa + A[11] * sina;
  
    A[8] = A[8] * cosa - T[0] * sina;
    A[9] = A[9] * cosa - T[1] * sina;
    A[10] = A[10] * cosa - T[2] * sina;
    A[11] = A[11] * cosa - T[3] * sina;
  
    return A;
}

/**
 
  Equivalent to matrix_multiply(A, rotation_y(tmp, angle)).
  This is faster because two rows remain unchanged by the rotation in matrix A.
 
 **/
 
static GLfloat *rotate_y(GLfloat *A, float angle)
{
    GLfloat cosa = cos(angle);
    GLfloat sina = sin(angle);
    GLfloat T[4];
  
    T[0] = A[0];
    T[1] = A[1];
    T[2] = A[2];
    T[3] = A[3];
  
    A[0] = T[0] * cosa - A[8] * sina;
    A[1] = T[1] * cosa - A[9] * sina;
    A[2] = T[2] * cosa - A[10] * sina;
    A[3] = T[3] * cosa - A[11] * sina;
  
    A[8] = T[0] * sina + A[8] * cosa;
    A[9] = T[1] * sina + A[9] * cosa;
    A[10] = T[2] * sina + A[10] * cosa;
    A[11] = T[3] * sina + A[11] * cosa;
  
    return A;
}

/**
 
  Equivalent to matrix_multiply(A, rotation_z(tmp, angle)).
  This is faster because two rows remain unchanged by the rotation in matrix A.
 
 **/
 
static GLfloat *rotate_z(GLfloat *A, float angle)
{
    GLfloat cosa = cos(angle);
    GLfloat sina = sin(angle);
    GLfloat T[4];
  
    T[0] = A[0];
    T[1] = A[1];
    T[2] = A[2];
    T[3] = A[3];
  
    A[0] = T[0] * cosa + A[4] * sina;
    A[1] = T[1] * cosa + A[5] * sina;
    A[2] = T[2] * cosa + A[6] * sina;
    A[3] = T[3] * cosa + A[7] * sina;
  
    A[4] = A[4] * cosa - T[0] * sina;
    A[5] = A[5] * cosa - T[1] * sina;
    A[6] = A[6] * cosa - T[2] * sina;
    A[7] = A[7] * cosa - T[3] * sina;
  
    return A;
}

/**
 
  Creates a translation matrix as per OpenGL definition of Translatef.
 
 **/
 
static GLfloat *translation(GLfloat *M, float x, float y, float z)
{
    M[0] = Identity[0];
    M[1] = Identity[1];
    M[2] = Identity[2];
    M[3] = Identity[3];
    M[4] = Identity[4];
    M[5] = Identity[5];
    M[6] = Identity[6];
    M[7] = Identity[7];
    M[8] = Identity[8];
    M[9] = Identity[9];
    M[10] = Identity[10];
    M[11] = Identity[11];
    M[12] = x;
    M[13] = y;
    M[14] = z;
    M[15] = 1.0f;
    return M;
}

/**
 
  Creates a view matrix that represents the camera's position and orientation.
  Unlike the modelview matrix, which might change for each draw call, the
  view matrix can only have one value per iteration. In this demo, the view matrix
  only changes when the user hits the arrow keys or the 'z' and 'Z' keys.
 
 **/
 
static void viewmatrix(void)
{
    translation(ViewMatrix, 0.0f, 0.0f, -40.0f);
    rotate_x(ViewMatrix, view_rotx);
    rotate_y(ViewMatrix, view_roty);
    rotate_z(ViewMatrix, view_rotz);
}
/**
 
  Changes for ES 2.0 compliance:
 
    * GLEW is used to access OpenGL 1.5 and 2.1 functions in Windows.
 
    * A program needs to be created to replace the fixed-function pipeline.
      The program has a single vertex shader and a single fragment shader.
      The vertex shader is read from the gears.vs file and the fragment
      shader is read from the gears.fs file.
 
    * Fixed function lighting is not supported as per ES 2.0 spec para 2.14.
      The enabling of GL_LIGHTING and GL_LIGHT0 is not necessary.
      The light position is store in a user-defined uniform in the vertex shader
      instead of using the pre-defined gl_LightSource[0]'s position member.
 
    * Display lists are not supported as per ES 2.0 spec paras 2.4 and 5.4.
      Immediate mode is not supported as per ES 2.0 spec para 2.6.
      Two buffers are created to hold the vertex data and indices respectively.
 
    * ES 2.0 only supports generic vertex attributes as per spec para 2.6.
 
 **/
 
void gears_init()
{
    static const float pos[3] = { 0.4082f, 0.4082f, 0.8165f };
  
    GLuint pos_index = 0, norm_index = 1;
    GLintptr offset;
  
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  
    load_program();
    pos_index = glGetAttribLocation(program, "Vertex");
    norm_index = glGetAttribLocation(program, "Normal");
    glUniform3fv(light_pos_loc, 1, pos);
  
    gear(1.0, 4.0, 1.0, 20, 0.7, 0, &gear1);
    gear(0.5, 2.0, 2.0, 10, 0.7, gear1.vcount, &gear2);
    gear(1.3, 2.0, 0.5, 10, 0.7, gear1.vcount + gear2.vcount, &gear3);
  
    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
  
    glBufferData(GL_ARRAY_BUFFER, (gear1.vcount + gear2.vcount + gear3.vcount) * sizeof(struct vertex), NULL, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (gear1.icount + gear2.icount + gear3.icount) * sizeof(GLushort), NULL, GL_STATIC_DRAW);
  
    offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, gear1.vcount * sizeof(struct vertex), gear1.vertices);
    offset += gear1.vcount * sizeof(struct vertex);
    glBufferSubData(GL_ARRAY_BUFFER, offset, gear2.vcount * sizeof(struct vertex), gear2.vertices);
    offset += gear2.vcount * sizeof(struct vertex);
    glBufferSubData(GL_ARRAY_BUFFER, offset, gear3.vcount * sizeof(struct vertex), gear3.vertices);
  
    offset = 0;
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, gear1.icount * sizeof(GLushort), gear1.indices);
    offset += gear1.icount * sizeof(GLushort);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, gear2.icount * sizeof(GLushort), gear2.indices);
    offset += gear2.icount * sizeof(GLushort);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, gear3.icount * sizeof(GLushort), gear3.indices);
  
    free(gear1.vertices);
    free(gear1.indices);
    free(gear2.vertices);
    free(gear2.indices);
    free(gear3.vertices);
    free(gear3.indices);
  
    glEnableVertexAttribArray(pos_index);
    glVertexAttribPointer(pos_index, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (const GLvoid *)0);
    glEnableVertexAttribArray(norm_index);
    glVertexAttribPointer(norm_index, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (const GLvoid *)(3 * sizeof(float)));
}

/**
 
  Changes for ES 2.0 compliance:
 
    * Display lists are not supported as per ES 2.0 spec paras 2.4 and 5.4.
      They were replaced by DrawElements with the data and indices sourced from VBOs.
      DrawRangeElements is not supported as per ES 2.0 spec para 2.8.
      The material calls saved in the original display lists must be performed
      before each DrawElements, since each gear has a different color.
 
    * Fixed function lighting is not supported as per ES 2.0 spec para 2.14.
      The material calls have been replaced by a single user-defined uniform.
      The product of the light model's ambient value with the material is
      pre-computed and stored in a uniform to replace gl_FrontLightModelProduct's
      ambient component.
 
    * Alpha is omitted from the materials because neither GL_ALPHA_TEST or
      GL_BLEND is enabled by default.
 
    * The fixed function transformation pipeline is not supported as per ES 2.0 spec para 2.11.
      The matrix operations were replaced by a local implementation.
      The stack approach was not used because it is not efficient.
      The ModelViewProjectionMatrix and NormalMatrix uniforms replace the pre-defined
      gl_ModelViewProjectionMatrix and gl_NormalMatrix uniforms. The other pre-defined
      matrices are not calculated since they are not used by the shaders.
 
 **/
 
void gears_draw()
{
    static const GLfloat lmred[4] = { 0.16f, 0.02f, 0.0f, 1.0f };
    static const GLfloat lmgreen[4] = { 0.0f, 0.16f, 0.04f, 1.0f };
    static const GLfloat lmblue[4] = { 0.04f, 0.04f, 0.2f, 1.0f };
  
    static const GLfloat red[4] = { 0.8f, 0.1f, 0.0f, 1.0f };
    static const GLfloat green[4] = { 0.0f, 0.8f, 0.2f, 1.0f };
    static const GLfloat blue[4] = { 0.2f, 0.2f, 1.0f, 1.0f };
  
    viewmatrix();
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    translation(ModelViewProjectionMatrix, -3.0, -2.0, 0.0);
    matrix_rmultiply(ViewMatrix, ModelViewProjectionMatrix);
    rotate_z(ModelViewProjectionMatrix, angle);
    matrix_copy_3x3((float*)NormalMatrix, ModelViewProjectionMatrix);
    matrix_rmultiply(ProjectionMatrix, ModelViewProjectionMatrix);
  
#if 0
    ModelViewProjectionMatrix[0] = 5.0f;
    ModelViewProjectionMatrix[1] = 0.0f;
    ModelViewProjectionMatrix[2] = 0.0f;
    ModelViewProjectionMatrix[3] = 0.0f;
    ModelViewProjectionMatrix[4] = 0.0f;
    ModelViewProjectionMatrix[5] = 6.25f;
    ModelViewProjectionMatrix[6] = 0.0;
    ModelViewProjectionMatrix[7] = 0.0f;
    ModelViewProjectionMatrix[8] = 0.0f;
    ModelViewProjectionMatrix[9] = 0.0f;
    ModelViewProjectionMatrix[10] = -1.18182;
    ModelViewProjectionMatrix[11] = -1.0f;
    ModelViewProjectionMatrix[12] = 0.0f;
    ModelViewProjectionMatrix[13] = 0.0f;
    ModelViewProjectionMatrix[14] = -10.9091f;
    ModelViewProjectionMatrix[15] = 0.0f;
#endif
  
    glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)ModelViewProjectionMatrix);
    glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, (GLfloat*)NormalMatrix);
    glUniform3fv(ambient_diffuse_loc, 1, (GLfloat*)&red);
    glUniform3fv(lm_prod_loc, 1, (GLfloat*)lmred);
  
    glDrawElements(GL_TRIANGLES, gear1.icount, GL_UNSIGNED_SHORT, (const GLvoid*)0);
  
    translation(ModelViewProjectionMatrix, 3.1, -2.0, 0.0);
    matrix_rmultiply(ViewMatrix, ModelViewProjectionMatrix);
    rotate_z(ModelViewProjectionMatrix, -2.0 * angle - 9.0 * deg2rad);
    matrix_copy_3x3((float*)NormalMatrix, ModelViewProjectionMatrix);
    matrix_rmultiply(ProjectionMatrix, ModelViewProjectionMatrix);
  
    glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)ModelViewProjectionMatrix);
    glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, (GLfloat*)NormalMatrix);
    glUniform3fv(ambient_diffuse_loc, 1, (GLfloat*)&green);
    glUniform3fv(lm_prod_loc, 1, (GLfloat*)lmgreen);
  
    glDrawElements(GL_TRIANGLES, gear2.icount, GL_UNSIGNED_SHORT, (const GLvoid*)(gear1.icount * sizeof(GLushort)));
  
    translation(ModelViewProjectionMatrix, -3.1, 4.2, 0.0);
    matrix_rmultiply(ViewMatrix, ModelViewProjectionMatrix);
    rotate_z(ModelViewProjectionMatrix, -2.0 * angle - 25.0 * deg2rad);
    matrix_copy_3x3((float*)NormalMatrix, ModelViewProjectionMatrix);
    matrix_rmultiply(ProjectionMatrix, ModelViewProjectionMatrix);
  
    glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)ModelViewProjectionMatrix);
    glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, (GLfloat*)NormalMatrix);
    glUniform3fv(ambient_diffuse_loc, 1, (GLfloat*)&blue);
    glUniform3fv(lm_prod_loc, 1, (GLfloat*)lmblue);
  
    glDrawElements(GL_TRIANGLES, gear3.icount, GL_UNSIGNED_SHORT, (const GLvoid*)((gear1.icount + gear2.icount) * sizeof(GLushort)));
}

/* OpenGL draw function & timing */
static void draw(void)
{
    glClearColor(0.0, 0.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gears_draw();
}

/* update animation parameters */
static void animate(void)
{
    angle = 3.f * (float) glfwGetTime();
}

/* change view angle, exit upon ESC */
void key( GLFWwindow* window, int k, int s, int action, int mods )
{
  if( action != GLFW_PRESS ) return;

  switch (k) {
  case GLFW_KEY_Z:
    if( mods & GLFW_MOD_SHIFT )
      view_rotz -= 5.0;
    else
      view_rotz += 5.0;
    break;
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    break;
  case GLFW_KEY_UP:
    view_rotx += 5.0;
    break;
  case GLFW_KEY_DOWN:
    view_rotx -= 5.0;
    break;
  case GLFW_KEY_LEFT:
    view_roty += 5.0;
    break;
  case GLFW_KEY_RIGHT:
    view_roty -= 5.0;
    break;
  default:
    return;
  }
}

/* new window size */
void reshape( GLFWwindow* window, int width, int height )
{
    GLfloat h = (GLfloat) height / (GLfloat) width;
    
    glViewport( 0, 0, (GLint) width, (GLint) height );
    frustum(-1.0f, 1.0f, -h, h, 5.0f, 60.0f);
}


/* program & OpenGL initialization */
static void init(void)
{
    gears_init();
}

#include <sys/time.h>

/* count fps to chech whether vsink works or not */
static void count_fps()
{
    static int frames = 0;
    static double lastDt = 0.0;
    double currentDt;
    double dt;
    
    ++frames;
    currentDt = glfwGetTime();
    if( !lastDt ){
        lastDt = currentDt;
        frames = 0;
    }else{
        dt = currentDt - lastDt;
        if( dt >= 1.0 ){
            printf( "FPS: %i\n", frames );
            lastDt = currentDt;
            frames = 0;
        }
    }
}

/* program entry */
int main(int argc, char *argv[])
{
    GLFWwindow* window;
    int width, height;
    
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);

    window = glfwCreateWindow( 300, 300, "Gears GLES2", glfwGetPrimaryMonitor(), NULL );
    if (!window)
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }
    glfwSetWindowPos( window, 100, 100 );

    // Set callback functions
    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval( 1 );

    glfwGetFramebufferSize(window, &width, &height);
    reshape(window, width, height);

    // Init gl structures
    init();

    // Main loop
    while( !glfwWindowShouldClose(window) )
    {
        // Draw gears
        draw();

        // Update animation
        animate();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        count_fps();
    }

    // Terminate GLFW
    glfwTerminate();

    // Exit program
    exit( EXIT_SUCCESS );
}
