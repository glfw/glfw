/*****************************************************************************
 * Title:   OpenGL ES Boing
 * Desc:    Tribute to Amiga Boing.
 * Author:  Jim Brooks  <gfx@jimbrooks.org>
 *          Original Amiga authors were R.J. Mical and Dale Luck.
 *          GLFW conversion by Marcus Geelnard
 *          OpenGL ES port by Mike Gorchak
 * Notes:   - 360' = 2*PI [radian]
 *
 *          - Distances between objects are created by doing a relative
 *            Z translations.
 *
 *          - Although OpenGL ES enticingly supports alpha-blending,
 *            the shadow of the original Boing didn't affect the color
 *            of the grid.
 *
 *          - [Marcus] Changed timing scheme from interval driven to frame-
 *            time based animation steps (which results in much smoother
 *            movement)
 *
 * History of Amiga Boing:
 *
 * Boing was demonstrated on the prototype Amiga (codenamed "Lorraine") in
 * 1985. According to legend, it was written ad-hoc in one night by
 * R. J. Mical and Dale Luck. Because the bouncing ball animation was so fast
 * and smooth, attendees did not believe the Amiga prototype was really doing
 * the rendering. Suspecting a trick, they began looking around the booth for
 * a hidden computer or VCR.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math.h>

#define GLAD_GLES2_IMPLEMENTATION
#include <glad/gles2.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <linmath.h>

/*****************************************************************************
 * Various declarations and macros
 *****************************************************************************/

/* Draw ball, or its shadow */
typedef enum
{
   DRAW_BALL,
   DRAW_BALL_SHADOW
} DRAW_BALL_ENUM;

/* Prototypes */
void init(void);
void display(void);
void reshape(GLFWwindow* window, int w, int h);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);
void GenerateBoingBall(void);
void GenerateGrid(void);
void DrawBoingBall(DRAW_BALL_ENUM drawBallHow);
void DrawGrid(void);
void BounceBall(float dt);

#define RADIUS           70.0f
#define STEP_LONGITUDE   22.5f                   /* 22.5 makes 8 bands like original Boing */
#define STEP_LATITUDE    22.5f

#define DIST_BALL        (RADIUS * 2.0f + RADIUS * 0.1f)

#define VIEW_SCENE_DIST  (DIST_BALL * 3.0f + 200.0f) /* distance from viewer to middle of boing area */
#define GRID_SIZE        (RADIUS * 4.5f)             /* length (width) of grid */
#define BOUNCE_HEIGHT    (RADIUS * 2.1f)
#define BOUNCE_WIDTH     (RADIUS * 2.1f)

#define SHADOW_OFFSET_X  -20.0f
#define SHADOW_OFFSET_Y  10.f
#define SHADOW_OFFSET_Z  0.0f

#define WALL_L_OFFSET    0.0f
#define WALL_R_OFFSET    5.0f

/* Animation speed (50.0 mimics the original GLUT demo speed) */
#define ANIMATION_SPEED  50.0f

/* Maximum allowed delta time per physics iteration */
#define MAX_DELTA_T      0.02f

/* Global vars */
int windowed_xpos, windowed_ypos, windowed_width, windowed_height;
int width, height;
int override_pos      = GLFW_FALSE;

float deg_rot_y       = 0.0f;
float deg_rot_y_inc   = 2.0f;
float cursor_x        = 0.0f;
float cursor_y        = 0.0f;
float ball_x          = -RADIUS;
float ball_y          = -RADIUS;
float ball_x_inc      = 1.0f;
float ball_y_inc      = 2.0f;
float t;
float t_old           = 0.0f;
float dt;

/* Random number generator */
#ifndef RAND_MAX
   #define RAND_MAX 4095
#endif

static mat4x4 projection;
static mat4x4 modelview_orig;
static mat4x4 modelview;
static mat4x4 mvp;

static vec3* ball_color1_vertices = NULL;
static vec3* ball_color2_vertices = NULL;
static vec3* grid_vertices = NULL;
static vec3 color1 = {0.8f, 0.1f, 0.1f};            /* reddish      */
static vec3 color2 = {0.95f, 0.95f, 0.95f};         /* almost white */
static vec3 color_shadow = {0.35f, 0.35f, 0.35f};   /* dark grey    */
static vec3 grid_color = {0.6f, 0.1f, 0.6f};        /* purple       */

static int facet_array_size = 0;
static int grid_array_size = 0;

static GLuint buffer;
static GLuint program;
static GLuint mvp_matrix_loc = -1;
static GLuint vertex_color_loc = -1;
static GLuint vertex_attr_index = -1;

/*****************************************************************************
 * Truncate a degree.
 *****************************************************************************/
float TruncateDeg(float deg)
{
   if (deg >= 360.0f)
      return (deg - 360.0f);
   else
      return deg;
}

/*****************************************************************************
 * Convert a degree (360-based) into a radian.
 * 360' = 2 * PI
 *****************************************************************************/
float deg2rad(float deg)
{
   return deg / 360.0f * (2.0f * M_PI);
}

/*****************************************************************************
 * 360' sin().
 *****************************************************************************/
float sin_deg(float deg)
{
   return sin(deg2rad(deg));
}

/*****************************************************************************
 * 360' cos().
 *****************************************************************************/
float cos_deg(float deg)
{
   return cos(deg2rad(deg));
}

/*****************************************************************************
 * init_shaders()
 *****************************************************************************/
static int init_shaders(void)
{
   GLenum error;
   GLuint vertex_shader, fragment_shader;
   char log[8192];
   GLsizei len;
   GLint processing_done = GL_TRUE;

   error = glGetError();

   const char *vtx_shdr_src =
      "attribute vec3 vertex_position;                            \n"
      "                                                           \n"
      "uniform mat4 mvp_matrix;                                   \n"
      "uniform vec4 vertex_color;                                 \n"
      "                                                           \n"
      "varying vec4 fs_color;                                     \n"
      "                                                           \n"
      "void main()                                                \n"
      "{                                                          \n"
      "    fs_color = vertex_color;                               \n"
      "    gl_Position = mvp_matrix * vec4(vertex_position, 1.0); \n"
      "}                                                          \n";

   const char *frg_shdr_src =
      "varying vec4 fs_color;                                     \n"
      "                                                           \n"
      "void main(void)                                            \n"
      "{                                                          \n"
      "    gl_FragColor = fs_color;                               \n"
      "}                                                          \n";

   program = glCreateProgram();

   vertex_shader = glCreateShader(GL_VERTEX_SHADER);
   glAttachShader(program, vertex_shader);
   glShaderSource(vertex_shader, 1, (const char**)&vtx_shdr_src, NULL);
   glCompileShader(vertex_shader);
   glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &processing_done);
   if (processing_done != GL_TRUE)
   {
      glGetShaderInfoLog(vertex_shader, sizeof(log), &len, log);
      fprintf(stderr, "Vertex Shader: %s", log);
      return -1;
   }

   fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
   glAttachShader(program, fragment_shader);
   glShaderSource(fragment_shader, 1, (const char**)&frg_shdr_src, NULL);
   glCompileShader(fragment_shader);
   glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &processing_done);
   if (processing_done != GL_TRUE)
   {
      glGetShaderInfoLog(fragment_shader, sizeof(log), &len, log);
      fprintf(stderr, "Fragment Shader: %s", log);
      return -1;
   }

   glLinkProgram(program);
   glGetProgramiv(program, GL_LINK_STATUS, &processing_done);
   if (processing_done != GL_TRUE)
   {
      glGetProgramInfoLog(program, sizeof(log), &len, log);
      fprintf(stderr, "Linker: %s", log);
      return -1;
   }

   glUseProgram(program);

   mvp_matrix_loc = glGetUniformLocation(program, "mvp_matrix");
   if ((mvp_matrix_loc == -1))
   {
      fprintf(stderr, "Can't locate location of MVP matrix uniform!\n");
      return -1;
   }

   vertex_color_loc = glGetUniformLocation(program, "vertex_color");
   if (vertex_color_loc == -1)
   {
      fprintf(stderr, "Can't locate location of fog color uniform!\n");
      return -1;
   }

   vertex_attr_index = glGetAttribLocation(program, "vertex_position");
   if (vertex_attr_index == -1)
   {
      fprintf(stderr, "Can't locate location of vertex attribute!\n");
      return -1;
   }

   /* We do not change VBO and use index offsets to the array elements */
   glEnableVertexAttribArray(vertex_attr_index);
   glVertexAttribPointer(vertex_attr_index, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(uintptr_t)0);

   error = glGetError();
   if (error != GL_NO_ERROR)
   {
      fprintf(stderr, "GL Error: 0x%04X in shaders setup\n", error);
      return -1;
   }

   return 0;
}

/*****************************************************************************
 * init()
 *****************************************************************************/
void init(void)
{
   int offset = 0;

   /* Clear background. */
   glClearColor(0.55f, 0.55f, 0.55f, 0.0f);

   /* Generate initial modelview matrix */
   vec3 eye = {0.0f, 0.0f, VIEW_SCENE_DIST};
   vec3 center = {0.0f, 0.0f, 0.0f};
   vec3 up = {0.0f, -1.0f, 0.0f};
   mat4x4_look_at(modelview_orig, eye, center, up);

   GenerateBoingBall();
   GenerateGrid();

   /* Setup VBO */
   glGenBuffers(1, &buffer);
   glBindBuffer(GL_ARRAY_BUFFER, buffer);
   glBufferData(GL_ARRAY_BUFFER, facet_array_size + facet_array_size + grid_array_size,
      NULL, GL_STATIC_DRAW);
   glBufferSubData(GL_ARRAY_BUFFER, offset, facet_array_size, ball_color1_vertices);
   offset += facet_array_size;
   glBufferSubData(GL_ARRAY_BUFFER, offset, facet_array_size, ball_color2_vertices);
   offset += facet_array_size;
   glBufferSubData(GL_ARRAY_BUFFER, offset, grid_array_size, grid_vertices);
   offset += grid_array_size;

   /* We don't need vertices in a heap, they were uploaded as VBO */
   free(ball_color1_vertices);
   free(ball_color2_vertices);
   free(grid_vertices);

   init_shaders();
}

/*****************************************************************************
 * display()
 *****************************************************************************/
void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   DrawBoingBall(DRAW_BALL_SHADOW);
   DrawGrid();
   DrawBoingBall(DRAW_BALL);
}

/*****************************************************************************
 * reshape()
 *****************************************************************************/
void reshape(GLFWwindow* window, int w, int h)
{
   width = w;
   height = h;

   glViewport(0, 0, (GLsizei)w, (GLsizei)h);

   mat4x4_perspective(projection, 2.0f * (float)atan2(RADIUS, 200.0f), (float)w / (float)h, 1.0f, VIEW_SCENE_DIST);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_ESCAPE && mods == 0)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if ((key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT) ||
        (key == GLFW_KEY_F11 && mods == GLFW_MOD_ALT))
    {
        if (glfwGetWindowMonitor(window))
        {
            glfwSetWindowMonitor(window, NULL,
                                 windowed_xpos, windowed_ypos,
                                 windowed_width, windowed_height, 0);
        }
        else
        {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if (monitor)
            {
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwGetWindowPos(window, &windowed_xpos, &windowed_ypos);
                glfwGetWindowSize(window, &windowed_width, &windowed_height);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
        }
    }
}

static void set_ball_pos(float x, float y)
{
   ball_x = (width / 2) - x;
   ball_y = y - (height / 2);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
   if (button != GLFW_MOUSE_BUTTON_LEFT)
      return;

   if (action == GLFW_PRESS)
   {
      override_pos = GLFW_TRUE;
      set_ball_pos(cursor_x, cursor_y);
   }
   else
   {
      override_pos = GLFW_FALSE;
   }
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
   cursor_x = (float)x;
   cursor_y = (float)y;

   if (override_pos)
      set_ball_pos(cursor_x, cursor_y);
}

/*****************************************************************************
 * Generate the Boing ball.
 *
 * The Boing ball is sphere in which each facet is a rectangle.
 * Facet colors alternate between red and white.
 * The ball is built by stacking latitudinal circles.  Each circle is composed
 * of a widely-separated set of points, so that each facet is noticeably large.
 *****************************************************************************/
void GenerateBoingBall(void)
{
   /* degree of longitude */
   float lon_deg;
   float lat_deg;
   int vertex_idx_c1 = 0;
   int vertex_idx_c2 = 0;
   bool colorToggle = 0;

   /* "ne" means south-east, so on */
   vec3 vert_ne;
   vec3 vert_nw;
   vec3 vert_sw;
   vec3 vert_se;

   /* "/ 2" - half facets of one color              */
   /* "* 6" - 6 vertices per facet (two triangles)  */
   facet_array_size = (int)((180.0f / STEP_LONGITUDE) * (360.0f / STEP_LATITUDE) / 2) * 6 * sizeof(vec3);
   ball_color1_vertices = calloc(1, facet_array_size);
   ball_color2_vertices = calloc(1, facet_array_size);
   if ((ball_color1_vertices == NULL) || (ball_color2_vertices == NULL))
   {
       fprintf(stderr, "Can't allocate memory for ball vertices!\n");
       glfwTerminate();
       exit(EXIT_FAILURE);
   }

   /*
    * Build a faceted latitude slice of the Boing ball,
    * stepping same-sized vertical bands of the sphere.
    */
   for (lon_deg = 0; lon_deg < 180; lon_deg += STEP_LONGITUDE)
   {
      /*
       * Iterate through the points of a latitude circle.
       * A latitude circle is a 2D set of X,Z points.
       */
      for (lat_deg = 0; lat_deg <= (360 - STEP_LATITUDE); lat_deg += STEP_LATITUDE)
      {
         /* Assign each Y. */
         vert_ne[1] = vert_nw[1] = (float) cos_deg((lon_deg + STEP_LONGITUDE)) * RADIUS;
         vert_sw[1] = vert_se[1] = (float) cos_deg(lon_deg) * RADIUS;

         /*
          * Assign each X,Z with sin,cos values scaled by latitude radius indexed by longitude.
          * Eg, long=0 and long=180 are at the poles, so zero scale is sin(longitude),
          * while long=90 (sin(90)=1) is at equator.
          */
         vert_ne[0] = (float)cos_deg(lat_deg                ) * (RADIUS * (float)sin_deg(lon_deg + STEP_LONGITUDE));
         vert_se[0] = (float)cos_deg(lat_deg                ) * (RADIUS * (float)sin_deg(lon_deg                 ));
         vert_nw[0] = (float)cos_deg(lat_deg + STEP_LATITUDE) * (RADIUS * (float)sin_deg(lon_deg + STEP_LONGITUDE));
         vert_sw[0] = (float)cos_deg(lat_deg + STEP_LATITUDE) * (RADIUS * (float)sin_deg(lon_deg                 ));

         vert_ne[2] = (float)sin_deg(lat_deg                ) * (RADIUS * (float)sin_deg(lon_deg + STEP_LONGITUDE));
         vert_se[2] = (float)sin_deg(lat_deg                ) * (RADIUS * (float)sin_deg(lon_deg                 ));
         vert_nw[2] = (float)sin_deg(lat_deg + STEP_LATITUDE) * (RADIUS * (float)sin_deg(lon_deg + STEP_LONGITUDE));
         vert_sw[2] = (float)sin_deg(lat_deg + STEP_LATITUDE) * (RADIUS * (float)sin_deg(lon_deg                 ));

         /*
          * Color this polygon with red or white. Replace polygons with
          * triangles to perform a batch draw operation.
          */
         if (colorToggle)
         {
            memcpy(ball_color1_vertices[vertex_idx_c1 + 0], vert_ne, sizeof(vec3)); /* V0 */
            memcpy(ball_color1_vertices[vertex_idx_c1 + 1], vert_nw, sizeof(vec3)); /* V1 */
            memcpy(ball_color1_vertices[vertex_idx_c1 + 2], vert_sw, sizeof(vec3)); /* V2 */
            memcpy(ball_color1_vertices[vertex_idx_c1 + 3], vert_ne, sizeof(vec3)); /* V0 */
            memcpy(ball_color1_vertices[vertex_idx_c1 + 4], vert_sw, sizeof(vec3)); /* V2 */
            memcpy(ball_color1_vertices[vertex_idx_c1 + 5], vert_se, sizeof(vec3)); /* V3 */
            vertex_idx_c1 += 6;
         }
         else
         {
            memcpy(ball_color2_vertices[vertex_idx_c2 + 0], vert_ne, sizeof(vec3)); /* V0 */
            memcpy(ball_color2_vertices[vertex_idx_c2 + 1], vert_nw, sizeof(vec3)); /* V1 */
            memcpy(ball_color2_vertices[vertex_idx_c2 + 2], vert_sw, sizeof(vec3)); /* V2 */
            memcpy(ball_color2_vertices[vertex_idx_c2 + 3], vert_ne, sizeof(vec3)); /* V0 */
            memcpy(ball_color2_vertices[vertex_idx_c2 + 4], vert_sw, sizeof(vec3)); /* V2 */
            memcpy(ball_color2_vertices[vertex_idx_c2 + 5], vert_se, sizeof(vec3)); /* V3 */
            vertex_idx_c2 += 6;
         }

         colorToggle = ! colorToggle;
      }

      /* Toggle color so that next band will opposite red/white colors than this one. */
      colorToggle = ! colorToggle;
   }
}

/*****************************************************************************
 * Bounce the ball.
 *****************************************************************************/
void BounceBall(float delta_t)
{
   float sign;
   float deg;

   if (override_pos)
     return;

   /* Bounce on walls */
   if (ball_x > (BOUNCE_WIDTH / 2 + WALL_R_OFFSET))
   {
      ball_x_inc = -0.5f - 0.75f * (float)rand() / (float)RAND_MAX;
      deg_rot_y_inc = -deg_rot_y_inc;
   }
   if (ball_x < -(BOUNCE_HEIGHT / 2 + WALL_L_OFFSET))
   {
      ball_x_inc = 0.5f + 0.75f * (float)rand() / (float)RAND_MAX;
      deg_rot_y_inc = -deg_rot_y_inc;
   }

   /* Bounce on floor / roof */
   if (ball_y > BOUNCE_HEIGHT / 2)
   {
      ball_y_inc = -0.75f - 1.0f * (float)rand() / (float)RAND_MAX;
   }
   if (ball_y < -BOUNCE_HEIGHT / 2 * 0.85)
   {
      ball_y_inc =  0.75f + 1.0f * (float)rand() / (float)RAND_MAX;
   }

   /* Update ball position */
   ball_x += ball_x_inc * ((float)delta_t * ANIMATION_SPEED);
   ball_y += ball_y_inc * ((float)delta_t * ANIMATION_SPEED);

   /* Simulate the effects of gravity on Y movement. */
   if (ball_y_inc < 0) sign = -1.0; else sign = 1.0;

   deg = (ball_y + BOUNCE_HEIGHT / 2) * 90 / BOUNCE_HEIGHT;
   if (deg > 80) deg = 80;
   if (deg < 10) deg = 10;

   ball_y_inc = sign * 4.0f * (float)sin_deg(deg);
}

/*****************************************************************************
 * Draw the ball.
 *****************************************************************************/
void DrawBoingBall(DRAW_BALL_ENUM drawBallHow)
{
   float dt_total, dt2;
   GLenum error;

   /* Reset error */
   error = glGetError();

   memcpy(modelview, modelview_orig, sizeof(modelview));

   /* Another relative Z translation to separate objects. */
   mat4x4_translate_in_place(modelview, 0.0f, 0.0f, DIST_BALL);

   /* Update ball position and rotation (iterate if necessary) */
   dt_total = dt;
   while (dt_total > 0.0)
   {
       dt2 = dt_total > MAX_DELTA_T ? MAX_DELTA_T : dt_total;
       dt_total -= dt2;
       BounceBall(dt2);
       deg_rot_y = TruncateDeg(deg_rot_y + deg_rot_y_inc * ((float)dt2 * ANIMATION_SPEED));
   }

   /* Set ball position */
   mat4x4_translate_in_place(modelview, ball_x, ball_y, 0.0f);

   /* Offset the shadow. */
   if (drawBallHow == DRAW_BALL_SHADOW)
   {
      mat4x4_translate_in_place(modelview, SHADOW_OFFSET_X, SHADOW_OFFSET_Y, SHADOW_OFFSET_Z);
   }

   /* Tilt the ball. */
   mat4x4_rotate(modelview, modelview, 0.0f, 0.0f, 1.0f, deg2rad(-20.0f));

   /* Continually rotate ball around Y axis. */
   mat4x4_rotate(modelview, modelview, 0.0f, 1.0f, 0.0f, deg2rad(deg_rot_y));

   /* Set OpenGL state for Boing ball. */
   glCullFace(GL_FRONT);
   glEnable(GL_CULL_FACE);

   mat4x4_mul(mvp, projection, modelview);
   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)mvp);

   if (drawBallHow == DRAW_BALL_SHADOW)
   {
       glUniform4fv(vertex_color_loc, 1, color_shadow);
   }
   else
   {
       glUniform4fv(vertex_color_loc, 1, color1);
   }

   glDrawArrays(GL_TRIANGLES, 0, facet_array_size / sizeof(vec3));

   if (drawBallHow == DRAW_BALL_SHADOW)
   {
       glUniform4fv(vertex_color_loc, 1, color_shadow);
   }
   else
   {
       glUniform4fv(vertex_color_loc, 1, color2);
   }

   glDrawArrays(GL_TRIANGLES, facet_array_size / sizeof(vec3), facet_array_size / sizeof(vec3));

   error = glGetError();
   if (error != GL_NO_ERROR)
   {
      fprintf(stderr, "GL Error: 0x%04X in ball draw function\n", error);
   }

   return;
}

/*****************************************************************************
 * Generate grid of lines
 *****************************************************************************/
void GenerateGrid(void)
{
   const int   rowTotal  = 12;                   /* must be divisible by 2 */
   const int   colTotal  = rowTotal;             /* must be same as rowTotal */
   const float widthLine = 2.0f;                 /* should be divisible by 2 */
   const float sizeCell  = GRID_SIZE / rowTotal;
   const float z_offset  = -40.0f;
   int         row, col;
   float       xl, xr;
   float       yt, yb;
   int         grid_idx = 0;

   /* "* 6" - 6 vertices per line (two triangles) */
   grid_array_size = ((rowTotal + 1) + (colTotal + 1)) * 6 * sizeof(vec3);
   grid_vertices = calloc(1, facet_array_size);
   if (grid_vertices == NULL)
   {
       fprintf(stderr, "Can't allocate memory for grid vertices!\n");
       glfwTerminate();
       exit(EXIT_FAILURE);
   }

   /* Generate vertical lines (as skinny 3D rectangles). */
   for (col = 0; col <= colTotal; col++)
   {
      /* Compute coords of line. */
      xl = -GRID_SIZE / 2 + col * sizeCell;
      xr = xl + widthLine;

      yt =  GRID_SIZE / 2;
      yb = -GRID_SIZE / 2 - widthLine;

      memcpy(grid_vertices[grid_idx + 0], (vec3){xr, yt, z_offset}, sizeof(vec3)); /* NE, V0 */
      memcpy(grid_vertices[grid_idx + 1], (vec3){xl, yt, z_offset}, sizeof(vec3)); /* NW, V1 */
      memcpy(grid_vertices[grid_idx + 2], (vec3){xl, yb, z_offset}, sizeof(vec3)); /* SW, V2 */
      memcpy(grid_vertices[grid_idx + 3], (vec3){xr, yt, z_offset}, sizeof(vec3)); /* NE, V0 */
      memcpy(grid_vertices[grid_idx + 4], (vec3){xl, yb, z_offset}, sizeof(vec3)); /* SW, V2 */
      memcpy(grid_vertices[grid_idx + 5], (vec3){xr, yb, z_offset}, sizeof(vec3)); /* SE, V3 */
      grid_idx += 6;
   }

   /* Generate horizontal lines (as skinny 3D rectangles). */
   for (row = 0; row <= rowTotal; row++)
   {
      /* Compute coords of line. */
      yt = GRID_SIZE / 2 - row * sizeCell;
      yb = yt - widthLine;

      xl = -GRID_SIZE / 2;
      xr =  GRID_SIZE / 2 + widthLine;

      memcpy(grid_vertices[grid_idx + 0], (vec3){xr, yt, z_offset}, sizeof(vec3)); /* NE, V0 */
      memcpy(grid_vertices[grid_idx + 1], (vec3){xl, yt, z_offset}, sizeof(vec3)); /* NW, V1 */
      memcpy(grid_vertices[grid_idx + 2], (vec3){xl, yb, z_offset}, sizeof(vec3)); /* SW, V2 */
      memcpy(grid_vertices[grid_idx + 3], (vec3){xr, yt, z_offset}, sizeof(vec3)); /* NE, V0 */
      memcpy(grid_vertices[grid_idx + 4], (vec3){xl, yb, z_offset}, sizeof(vec3)); /* SW, V2 */
      memcpy(grid_vertices[grid_idx + 5], (vec3){xr, yb, z_offset}, sizeof(vec3)); /* SE, V3 */
      grid_idx += 6;
   }
}

/*****************************************************************************
 * Draw the purple grid of lines, behind the Boing ball.
 * When the Workbench is dropped to the bottom, Boing shows 12 rows.
 *****************************************************************************/
void DrawGrid(void)
{
   memcpy(modelview, modelview_orig, sizeof(modelview));

   glDisable(GL_CULL_FACE);

   /* Another relative Z translation to separate objects. */
   mat4x4_translate_in_place(modelview, 0.0f, 0.0f, DIST_BALL);

   mat4x4_mul(mvp, projection, modelview);
   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)mvp);

   glUniform4fv(vertex_color_loc, 1, grid_color);
   glDrawArrays(GL_TRIANGLES, (facet_array_size + facet_array_size) / sizeof(vec3),
      (facet_array_size + facet_array_size) / sizeof(vec3));

   return;
}

/*======================================================================*
 * main()
 *======================================================================*/

int main(void)
{
   GLFWwindow* window;

   /* Init GLFW */
   if (!glfwInit())
      exit(EXIT_FAILURE);

   window = glfwCreateWindow(400, 400, "Boing OpenGL ES 2.x (classic Amiga demo)", NULL, NULL);
   if (!window)
   {
       glfwTerminate();
       exit(EXIT_FAILURE);
   }

   glfwSetWindowAspectRatio(window, 1, 1);

   glfwSetFramebufferSizeCallback(window, reshape);
   glfwSetKeyCallback(window, key_callback);
   glfwSetMouseButtonCallback(window, mouse_button_callback);
   glfwSetCursorPosCallback(window, cursor_position_callback);

   glfwMakeContextCurrent(window);
   gladLoadGLES2(glfwGetProcAddress);
   glfwSwapInterval(1);

   glfwGetFramebufferSize(window, &width, &height);
   reshape(window, width, height);

   glfwSetTime(0.0);

   init();

   /* Main loop */
   for (;;)
   {
       /* Timing */
       t = glfwGetTime();
       dt = t - t_old;
       t_old = t;

       /* Draw one frame */
       display();

       /* Swap buffers */
       glfwSwapBuffers(window);
       glfwPollEvents();

       /* Check if we are still running */
       if (glfwWindowShouldClose(window))
           break;
   }

   glfwTerminate();
   exit(EXIT_SUCCESS);
}
