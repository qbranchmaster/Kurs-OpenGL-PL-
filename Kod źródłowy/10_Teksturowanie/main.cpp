#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <FreeImage.h>

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
//******************************************************************************
int window_width;
int window_height;
GLuint shaders;
GLFWwindow* window;
//******************************************************************************
GLint LoadShaders(std::string vertex_shader, std::string fragment_shader)
{
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertex_shader_data;
    std::ifstream vertex_shader_file(vertex_shader.c_str(), std::ios::in);
    if (vertex_shader_file.is_open())
    {
        std::string line;
        while (std::getline(vertex_shader_file, line))
            vertex_shader_data += "\n" + line;

        vertex_shader_file.close();
    }

    std::string fragment_shader_data;
    std::ifstream fragment_shader_file(fragment_shader.c_str(), std::ios::in);
    if (fragment_shader_file.is_open())
    {
        std::string line;
        while (std::getline(fragment_shader_file, line))
            fragment_shader_data += "\n" + line;

        fragment_shader_file.close();
   }

   const char* vertex_ptr = vertex_shader_data.c_str();
   const char* fragment_ptr = fragment_shader_data.c_str();
   glShaderSource(vertex_shader_id, 1, &vertex_ptr, NULL);
   glShaderSource(fragment_shader_id, 1, &fragment_ptr, NULL);

   glCompileShader(vertex_shader_id);
   glCompileShader(fragment_shader_id);

   GLuint shader_programme = glCreateProgram();
   glAttachShader(shader_programme, vertex_shader_id);
   glAttachShader(shader_programme, fragment_shader_id);
   glLinkProgram(shader_programme);

   glDeleteShader(vertex_shader_id);
   glDeleteShader(fragment_shader_id);

   return shader_programme;
}
//******************************************************************************
static void ErrorCallback(int /*error*/, const char* description)
{
    std::cout << "GLFW Error: " << description << std::endl;
}
//******************************************************************************
static void KeyCallback(GLFWwindow* window, int key, int /*scancode*/,
                        int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
//******************************************************************************
static void WindowSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    window_width = width;
    window_height = height;
}
//******************************************************************************
int CreateWindow(int width, int height, std::string name, int samples,
                 bool fullscreen)
{
    glfwSetErrorCallback(ErrorCallback);

    if(!glfwInit())
        return -1;

    glfwWindowHint(GLFW_SAMPLES, samples);

    if (fullscreen)
    {
        const GLFWvidmode* video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window_width = video_mode->width;
        window_height = video_mode->height;
        window = glfwCreateWindow(window_width, window_height, name.c_str(),
                                  glfwGetPrimaryMonitor(), NULL);
    }
    else
    {
        window_width = width;
        window_height = height;
        window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
    }

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        return -1;

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetWindowSizeCallback(window, WindowSizeCallback);

    return 0;
}
//******************************************************************************
void EnableDepthTesting()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}
//******************************************************************************
void ProcessWindowEvents()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}
//******************************************************************************
void Terminate()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
//******************************************************************************
bool RenderingEnabled()
{
    if (glfwWindowShouldClose(window))
        return false;

    return true;
}
//******************************************************************************
void ClearColor(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window_width, window_height);
}
//******************************************************************************
void ActivateShaderProgram(GLuint shader_program)
{
    glUseProgram(shader_program);
}
//******************************************************************************
int LoadTexture(std::string file_name)
{
    FREE_IMAGE_FORMAT image_format = FIF_UNKNOWN;
    FIBITMAP* image_ptr = 0;
    BYTE* bits = 0;

    image_format = FreeImage_GetFileType(file_name.c_str(), 0);
    if(image_format == FIF_UNKNOWN)
       image_format = FreeImage_GetFIFFromFilename(file_name.c_str());

    if(FreeImage_FIFSupportsReading(image_format))
       image_ptr = FreeImage_Load(image_format, file_name.c_str());

    bits = FreeImage_GetBits(image_ptr);

    int image_width = 0;
    int image_height = 0;
    image_width = FreeImage_GetWidth(image_ptr);
    image_height = FreeImage_GetHeight(image_ptr);
    if((bits == 0) || (image_width == 0) || (image_height == 0))
        return -1;

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_BGR,
                 GL_UNSIGNED_BYTE, bits);
    glGenerateMipmap(GL_TEXTURE_2D);

    return 0;
}
//******************************************************************************
int main()
{
    int result = CreateWindow(640, 480, "GL Window", 4, false);
    if (result)
        return -1;

    GLfloat points[] = {
     -0.5f, -0.5f,  0.0f,
      0.5f, -0.5f,  0.0f,
      0.5f,  0.5f,  0.0f,
      0.5f,  0.5f,  0.0f,
     -0.5f,  0.5f,  0.0f,
     -0.5f, -0.5f,  0.0f,
    };

    GLfloat tex_coords[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
    };

    GLuint position_vbo = 0;
    glGenBuffers(1, &position_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    GLuint texture_coords_vbo = 0;
    glGenBuffers(1, &texture_coords_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, texture_coords_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, texture_coords_vbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    EnableDepthTesting();

    GLuint shaders = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    ActivateShaderProgram(shaders);
    GLint texture_slot = glGetUniformLocation(shaders, "basic_texture");
    glUniform1i(texture_slot, 0);

    glActiveTexture(GL_TEXTURE0);
    if (LoadTexture("Brick.jpg"))
        return -1;

    while (RenderingEnabled())
    {
        ClearColor(0.5, 0.5, 0.5);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ProcessWindowEvents();
    }

    Terminate();

    return 0;
}