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
    glfwSetKeyCallback(window, key_callback);
     
    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
 
    return 0;
}
