/* OpenGL example code - shader_image_load_store
 * 
 * This example solves the electromagnetic wave equation with a FDTD
 * scheme (finite difference time domain). Updates of the texture
 * representing the grid are done in place by use of image objects.
 * 
 * Autor: Jakob Progsch
 */

#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/noise.hpp> 

#include <iostream>
#include <string>
#include <vector>


// helper to check and display for shader compiler errors
bool check_shader_compile_status(GLuint obj) {
    GLint status;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;
    }
    return true;
}

// helper to check and display for shader linker error
bool check_program_link_status(GLuint obj) {
    GLint status;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;   
    }
    return true;
}

int main() {
    int width = 640;
    int height = 480;
    
    if(glfwInit() == GL_FALSE) {
        std::cerr << "failed to init GLFW" << std::endl;
        return 1;
    }

    // select opengl version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
    // create a window
    GLFWwindow *window;
    if((window = glfwCreateWindow(width, height, "12shader_image_load_store", 0, 0)) == 0) {
        std::cerr << "failed to open window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);

    if(glxwInit()) {
        std::cerr << "failed to init GL3W" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    glfwSwapInterval(1);
    

    // shader source code
    // shared vertex shader
    std::string vertex_source =
        "#version 420\n"
        "layout(location = 0) in vec4 vposition;\n"
        "void main() {\n"
        "   gl_Position = vposition;\n"
        "}\n";
    
    // the first fragment shader doesn't output anything since it only
    // updates the image in place
    std::string fragment1_source =
        "#version 420\n"
        "uniform float dt;\n"
        "uniform ivec2 image_size;\n"
        "uniform layout(rgba32f) image2D image;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   ivec2 coords = ivec2(gl_FragCoord.xy);\n"
        "	vec4 HE = imageLoad(image, coords);\n"
		"	float Ezdx = HE.z-imageLoad(image, coords-ivec2(1, 0)).z;\n"
		"	float Ezdy = HE.z-imageLoad(image, coords-ivec2(0, 1)).z;\n"
		"   HE.xy += dt*vec2(-Ezdy, Ezdx);\n"
        "   imageStore(image, coords, HE);\n"
        "}\n";
    
        
    // the second fragment shader also outputs the frag color for display
    // purposes
    std::string fragment2_source =
        "#version 420\n"
        "uniform float t;\n"
        "uniform float dt;\n"
        "uniform ivec2 image_size;\n"
        "uniform layout(rgba32f) image2D image;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   ivec2 coords = ivec2(gl_FragCoord.xy);\n"
		
        "	float e = 1;\n"
        "	vec4 HE = imageLoad(image, coords);\n"
		"	float r = HE.w;\n"
		"	float Hydx = imageLoad(image, coords+ivec2(1, 0)).y\n"
		"				-HE.y;\n"
		"	float Hxdy = imageLoad(image, coords+ivec2(0, 1)).x\n"
		"				-HE.x;\n"
		
		"	float Eout = dt*(Hydx-Hxdy)/(e);\n"
        "   HE.z = HE.z*(1-dt*r/e) + Eout;\n"
        
        // add source at image center
        "   if(coords.x == image_size.x/2 && coords.y == image_size.y/2) {\n"
        "   	HE.z += 30*sin(15*t)*exp(-20*(t-2)*(t-2));\n"
        "	}\n"
        
        "   imageStore(image, coords, HE);\n"
        "   FragColor = vec4(HE.z, HE.w, -HE.z, 1);\n"
        "}\n";
   
    // program and shader handles
    GLuint shader1_program, shader2_program, vertex_shader, fragment1_shader, fragment2_shader;
    
    // we need these to properly pass the strings
    const char *source;
    int length;

    // create and compiler vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    source = vertex_source.c_str();
    length = vertex_source.size();
    glShaderSource(vertex_shader, 1, &source, &length); 
    glCompileShader(vertex_shader);
    if(!check_shader_compile_status(vertex_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
 
    // create and compiler fragment shader
    fragment1_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragment1_source.c_str();
    length = fragment1_source.size();
    glShaderSource(fragment1_shader, 1, &source, &length);   
    glCompileShader(fragment1_shader);
    if(!check_shader_compile_status(fragment1_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // create and compiler fragment shader
    fragment2_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragment2_source.c_str();
    length = fragment2_source.size();
    glShaderSource(fragment2_shader, 1, &source, &length);   
    glCompileShader(fragment2_shader);
    if(!check_shader_compile_status(fragment2_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    // create program
    shader1_program = glCreateProgram();
    
    // attach shaders
    glAttachShader(shader1_program, vertex_shader);
    glAttachShader(shader1_program, fragment1_shader);
    
    // link the program and check for errors
    glLinkProgram(shader1_program);
    check_program_link_status(shader1_program);
    
    // get texture uniform location
    GLint image_size_location1 = glGetUniformLocation(shader1_program, "image_size");
    GLint image_location1 = glGetUniformLocation(shader1_program, "image");
    GLint dt_location1 = glGetUniformLocation(shader1_program, "dt");

    
    // create program
    shader2_program = glCreateProgram();
    
    // attach shaders
    glAttachShader(shader2_program, vertex_shader);
    glAttachShader(shader2_program, fragment2_shader);
    
    // link the program and check for errors
    glLinkProgram(shader2_program);
    check_program_link_status(shader2_program);
    
    // get texture uniform location
    GLint image_size_location2 = glGetUniformLocation(shader2_program, "image_size");
    GLint image_location2 = glGetUniformLocation(shader2_program, "image");
    GLint t_location2 = glGetUniformLocation(shader2_program, "t");
    GLint dt_location2 = glGetUniformLocation(shader2_program, "dt");

    // vao and vbo handle
    GLuint vao, vbo, ibo;
 
    // generate and bind the vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // generate and bind the vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
            
    // data for a fullscreen quad
    GLfloat vertexData[] = {
    //  X     Y     Z
       1.0f, 1.0f, 0.0f, // vertex 0
      -1.0f, 1.0f, 0.0f, // vertex 1
       1.0f,-1.0f, 0.0f, // vertex 2
      -1.0f,-1.0f, 0.0f, // vertex 3
    }; // 4 vertices with 3 components (floats) each

    // fill with data
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*3, vertexData, GL_STATIC_DRAW);
                    
           
    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

    
    // generate and bind the index buffer object
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            
    GLuint indexData[] = {
        0,1,2, // first triangle
        2,1,3, // second triangle
    };

    // fill with data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*2*3, indexData, GL_STATIC_DRAW);

    // texture handle
    GLuint texture;
    
    // generate texture
    glGenTextures(1, &texture);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // create some image data
    std::vector<GLfloat> image(4*width*height);
    for(int j = 0;j<height;++j) {
        for(int i = 0;i<width;++i) {
            size_t index = j*width + i;
            image[4*index + 0] = 0.0f;
            image[4*index + 1] = 0.0f;
            image[4*index + 2] = 0.0f;
            image[4*index + 3] = 20.0f*glm::clamp(glm::perlin(0.006f*glm::vec2(i,j+150)),0.0f,0.1f);
        }
    }
    
    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    
    // set texture content
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, &image[0]);
    
    float t = 0;
    float dt = 1.0f/60.0f;    
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        t += dt;
        
        // reset time every 10 seconds to repeat the sequence
        if(t>20) t = 0;

        // clear first
        glClear(GL_COLOR_BUFFER_BIT);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        
        // bind the vao
        glBindVertexArray(vao);

        int substeps = 5;
        
        glUseProgram(shader1_program);
            
        glUniform2i(image_size_location1, width, height);
        glUniform1i(image_location1, 0);
        glUniform1f(dt_location1, 50*dt/substeps);

        glUseProgram(shader2_program);

        glUniform2i(image_size_location2, width, height);
        glUniform1i(image_location2, 0);
        glUniform1f(dt_location2, 50*dt/substeps);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        int i = 0;
        for(;i<substeps-1;++i) {
            glUseProgram(shader1_program);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            
            glUseProgram(shader2_program);
            glUniform1f(t_location2, t+i*dt/substeps);
            
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        
        glUseProgram(shader1_program);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            
        glUseProgram(shader2_program);
        glUniform1f(t_location2, t+i*dt/substeps);
            
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
         
        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << error << std::endl;
            break;
        }
        
        // finally swap buffers
        glfwSwapBuffers(window);        
    }
    
    // delete the created objects
    
    glDeleteTextures(1, &texture);
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    
    glDetachShader(shader1_program, vertex_shader);	
    glDetachShader(shader1_program, fragment1_shader);
    glDetachShader(shader2_program, vertex_shader);	
    glDetachShader(shader2_program, fragment2_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment1_shader);
    glDeleteProgram(shader1_program);
    glDeleteShader(fragment2_shader);
    glDeleteProgram(shader2_program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

