// compute shaders tutorial
// Dr Anton Gerdelan <gerdela@scss.tcd.ie>
// Trinity College Dublin, Ireland
// 26 Feb 2016. latest v 2 Mar 2016

#include "gl_utils.h"

// this is the compute shader in an ugly C string
const char *compute_shader_str =
	"#version 430\n                                                               \
layout (local_size_x = 1, local_size_y = 1) in;\n                             \
layout (rgba32f, binding = 0) uniform image2D img_output;\n                   \
\n                                                                            \
void main () {\n                                                              \
  vec4 pixel = vec4 (0.0, 0.0, 0.0, 1.0);\n                                   \
  ivec2 pixel_coords = ivec2 (gl_GlobalInvocationID.xy);\n                    \
\n                                                                            \
float max_x = 5.0;\n                                                          \
float max_y = 5.0;\n                                                          \
ivec2 dims = imageSize (img_output);\n                                        \
float x = (float(pixel_coords.x * 2 - dims.x) / dims.x);\n                    \
float y = (float(pixel_coords.y * 2 - dims.y) / dims.y);\n                    \
vec3 ray_o = vec3 (x * max_x, y * max_y, 0.0);\n                              \
vec3 ray_d = vec3 (0.0, 0.0, -1.0); // ortho\n                                \
\n                                                                            \
vec3 sphere_c = vec3 (0.0, 0.0, -10.0);                                       \
float sphere_r = 1.0;                                                         \
\n                                                                            \
vec3 omc = ray_o - sphere_c;\n                                                \
float b = dot (ray_d, omc);\n                                                 \
float c = dot (omc, omc) - sphere_r * sphere_r;\n                             \
float bsqmc = b * b - c;\n                                                    \
float t = 10000.0;\n                                                          \
// hit one or both sides\n                                                    \
if (bsqmc >= 0.0) {\n                                                         \
  pixel = vec4 (0.4, 0.4, 1.0, 1.0);\n                                        \
}\n                                                                           \
\n                                                                            \
  imageStore (img_output, pixel_coords, pixel);\n                             \
}\n";

int main() {
	( start_gl() ); // just starts a 4.3 GL context+window

	// set up shaders and geometry for full-screen quad
	// moved code to gl_utils.cpp
	GLuint quad_vao = create_quad_vao();
	GLuint quad_program = create_quad_program();

	GLuint ray_program = 0;
	{ // create the compute shader
		GLuint ray_shader = glCreateShader( GL_COMPUTE_SHADER );
		glShaderSource( ray_shader, 1, &compute_shader_str, NULL );
		glCompileShader( ray_shader );
		( check_shader_errors( ray_shader ) ); // code moved to gl_utils.cpp
		ray_program = glCreateProgram();
		glAttachShader( ray_program, ray_shader );
		glLinkProgram( ray_program );
		( check_program_errors( ray_program ) ); // code moved to gl_utils.cpp
	}

	// texture handle and dimensions
	GLuint tex_output = 0;
	int tex_w = 512, tex_h = 512;
	{ // create the texture
		glGenTextures( 1, &tex_output );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, tex_output );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		// linear allows us to scale the window up retaining reasonable quality
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		// same internal format as compute shader input
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
									NULL );
		// bind to image unit so can write to specific pixels from the shader
		glBindImageTexture( 0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );
	}

	{ // query up the workgroups
		int work_grp_size[3], work_grp_inv;
		// maximum global work group (total work in a dispatch)
		glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_size[0] );
		glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_size[1] );
		glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_size[2] );
		printf( "max global (total) work group size x:%i y:%i z:%i\n", work_grp_size[0],
						work_grp_size[1], work_grp_size[2] );
		// maximum local work group (one shader's slice)
		glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0] );
		glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1] );
		glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2] );
		printf( "max local (in one shader) work group sizes x:%i y:%i z:%i\n",
						work_grp_size[0], work_grp_size[1], work_grp_size[2] );
		// maximum compute shader invocations (x * y * z)
		glGetIntegerv( GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv );
		printf( "max computer shader invocations %i\n", work_grp_inv );
	}

	while ( !glfwWindowShouldClose( window ) ) { // drawing loop
		{																					 // launch compute shaders!
			glUseProgram( ray_program );
			glDispatchCompute( (GLuint)tex_w, (GLuint)tex_h, 1 );
		}

		// prevent sampling befor all writes to image are done
		glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

		glClear( GL_COLOR_BUFFER_BIT );
		glUseProgram( quad_program );
		glBindVertexArray( quad_vao );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, tex_output );
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

		glfwPollEvents();
		if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( window, 1 );
		}
		glfwSwapBuffers( window );
	}

	stop_gl(); // stop glfw, close window
	return 0;
}
