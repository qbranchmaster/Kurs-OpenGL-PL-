#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
//******************************************************************************
GLFWwindow* window;
GLuint shaders1;
GLuint shaders2;
GLuint shaders3;
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
int CreateWindow(int width, int height, std::string name)
{
    glfwSetErrorCallback(ErrorCallback);

    if(!glfwInit())
        return -1;

    window = glfwCreateWindow(width, height, name.c_str(), NULL,
                                          NULL);
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

    return 0;
}
//******************************************************************************
int main()
{
    int result = CreateWindow(640, 480, "GL Window");
    if (result)
        return -1;

    GLuint shaders1 = LoadShaders("vertex_shader1.glsl", "fragment_shader1.glsl");
    GLuint shaders2 = LoadShaders("vertex_shader2.glsl", "fragment_shader2.glsl");
    GLuint shaders3 = LoadShaders("vertex_shader3.glsl", "fragment_shader3.glsl");

    GLfloat points[] = {
            -0.5f, -0.5f, 0.0f,
             0.0f, 0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    GLuint vao1 = 0;
    glGenVertexArrays(1, &vao1);
    glBindVertexArray(vao1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    GLuint vao2 = 0;
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    GLuint vao3 = 0;
    glGenVertexArrays(1, &vao3);
    glBindVertexArray(vao3);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaders1);
        glBindVertexArray(vao1);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shaders2);
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shaders3);
        glBindVertexArray(vao3);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
