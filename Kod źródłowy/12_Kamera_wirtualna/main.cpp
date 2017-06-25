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
void FPSCounter(double& fps)
{
    static double prev_time = glfwGetTime();
    double actual_time = glfwGetTime();

    static int frames_counter = 0;

    double elapsed_time = actual_time - prev_time;
    if (elapsed_time >= 1.0)
    {
        prev_time = actual_time;

        fps = static_cast<double>(frames_counter) / elapsed_time;

        frames_counter = 0;
    }

    frames_counter++;
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

    GLint trans_uniform = glGetUniformLocation(shaders, "trans_matrix");
    if(trans_uniform == -1)
        std::cout << "Variable 'trans_matrix' not found." << std::endl;

    GLint view_uniform = glGetUniformLocation(shaders, "view_matrix");
    if(view_uniform == -1)
        std::cout << "Variable 'view_matrix' not found." << std::endl;

    GLint perspective_uniform = glGetUniformLocation(shaders, "perspective_matrix");
    if(perspective_uniform == -1)
        std::cout << "Variable 'perspective_matrix' not found." << std::endl;

    glActiveTexture(GL_TEXTURE0);
    if (LoadTexture("Brick.jpg"))
        return -1;

    // View
    glm::mat4 view_matrix;
    glm::vec3 camera_pos(0.0, 0.0, 2.0);
    glm::vec3 direction(0.0, 0.0, -1.0);
    glm::vec3 right(1.0, 0.0, 0.0);
    glm::vec3 up = glm::cross(right, direction);

    // Metoda 1
    /*view_matrix[0] = glm::vec4(right.x, up.x, -direction.x, 0);
    view_matrix[1] = glm::vec4(right.y, up.y, -direction.y, 0);
    view_matrix[2] = glm::vec4(right.z, up.z, -direction.z, 0);
    view_matrix[3] = glm::vec4(-camera_pos.x, -camera_pos.y, -camera_pos.z, 1);*/

    // Metoda 2
    /*glm::mat4 t = glm::translate(glm::mat4(1.0), -camera_pos);
    glm::mat4 r = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0, 1, 0));
    view_matrix = r * t;*/

    // Metoda 3
    view_matrix = glm::lookAt(camera_pos, camera_pos + direction, up);

    // Perspective
    float P1 = 0.1f;
    float P2 = 100.0f;
    float FOV = 90.0f;
    float aspect = 16.0f / 9.0f;

    float D = tan(0.5 * FOV * 3.14 / 180.0) * P1;
    float Sx = (2.0 * P1) / (2 * D * aspect);
    float Sy = P1 / D;
    float Sz = -(P2 + P1) / (P2 - P1);
    float Pz = -(2.0f * P2 * P1) / (P2 - P1);

    glm::mat4 perspective;

    // Metoda 1
    /*perspective[0] = glm::vec4(Sx, 0, 0, 0);
    perspective[1] = glm::vec4(0, Sy, 0, 0);
    perspective[2] = glm::vec4(0, 0, Sz, -1.0);
    perspective[3] = glm::vec4(0, 0, Pz, 0);*/

    // Metoda 2
    perspective = glm::perspective(FOV, aspect, P1, P2);

    // Transform
    glm::mat4 transform_matrix;
    transform_matrix = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0, 1, 0));

    glUniformMatrix4fv(trans_uniform, 1, GL_FALSE, glm::value_ptr(transform_matrix));
    glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(perspective_uniform, 1, GL_FALSE, glm::value_ptr(perspective));

    while (RenderingEnabled())
    {
        static double fps = 0;
        FPSCounter(fps);
        std::string title = "GL Window @ FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window, title.c_str());

        ClearColor(0.5, 0.5, 0.5);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ProcessWindowEvents();
    }

    Terminate();

    return 0;
}