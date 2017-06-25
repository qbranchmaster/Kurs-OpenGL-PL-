#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

static void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
        return 1;

    GLFWwindow* window = glfwCreateWindow(640, 480, "My GLFW Window", NULL, NULL);
    if(!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    GLenum error_code = glewInit();
    if(error_code != GLEW_OK)
    {
        std::cerr << "GLEW init error: " << glewGetErrorString(error_code);
    }

    glfwSetKeyCallback(window, key_callback);

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* shading = glGetString(GL_SHADING_LANGUAGE_VERSION);

    std::cout << vendor << std::endl << renderer << std::endl << version << std::endl << shading << std::endl;

    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
