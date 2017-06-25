#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <FreeImage.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//******************************************************************************
int window_width;
int window_height;
GLFWwindow* window;

GLuint shaders;

glm::vec3 camera_position(0.0, 3.0, 10.0);
glm::vec3 camera_direction(1.0, 0.0, 0.0);
glm::vec3 camera_right(1.0, 0.0, 0.0);
glm::vec3 camera_up;
glm::mat4 view_matrix;
glm::mat4 perspective;
double horizontal_angle = 0;
double vertical_angle = 0;

float P1 = 0.1f;
float P2 = 100.0f;
float FOV = 1.15f;
float aspect;

double actual_time;
double previous_time;
//******************************************************************************
double GetTimeDelta()
{
    return actual_time - previous_time;
}
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
void ErrorCallback(int /*error*/, const char* description)
{
    std::cout << "GLFW Error: " << description << std::endl;
}
//******************************************************************************
void KeyCallback(GLFWwindow* window, int key, int /*scancode*/,
                        int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
        return;
    }

    float move_speed = 500.0f;

    if (key == GLFW_KEY_W)
        camera_position += camera_direction * (float)GetTimeDelta() * move_speed;
    if (key == GLFW_KEY_S)
        camera_position -= camera_direction * (float)GetTimeDelta() * move_speed;
    if (key == GLFW_KEY_A)
        camera_position -= camera_right * (float)GetTimeDelta() * move_speed;
    if (key == GLFW_KEY_D)
        camera_position += camera_right * (float)GetTimeDelta() * move_speed;

    view_matrix = glm::lookAt(camera_position, camera_position +
                              camera_direction, camera_up);
}
//******************************************************************************
void CursorPositionCallback(GLFWwindow* window, double x_cursor_pos,
                            double y_cursor_pos)
{
    glfwSetCursorPos(window, window_width / 2, window_height / 2);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float mouse_speed = 0.1f;

    horizontal_angle += mouse_speed * (float)GetTimeDelta() *
                        float(window_width / 2.0 - x_cursor_pos);
    vertical_angle += mouse_speed * (float)GetTimeDelta() *
                      float(window_height / 2.0 - y_cursor_pos);

    if (vertical_angle < -1.57)
        vertical_angle = -1.57;
    if (vertical_angle > 1.57)
        vertical_angle = 1.57;

    camera_direction = glm::vec3(cos(vertical_angle) * sin(horizontal_angle),
                                 sin(vertical_angle),
                                 cos(vertical_angle) * cos(horizontal_angle));
    camera_right = glm::vec3(-cos(horizontal_angle), 0,
                             sin(horizontal_angle));
    camera_up = glm::cross(camera_right, camera_direction);

    view_matrix = glm::lookAt(camera_position, camera_position +
                              camera_direction, camera_up);
}
//******************************************************************************
void WindowSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    window_width = width;
    window_height = height;

    aspect = float(window_width) / float(window_height);
    perspective = glm::perspective(FOV, aspect, P1, P2);
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
    glfwSetCursorPosCallback(window, CursorPositionCallback);
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
void EnableFaceCulling()
{
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
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
int LoadTexture(std::string file_name, GLuint& texture_handle)
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

    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    unsigned int colours = FreeImage_GetBPP(image_ptr);
    if (colours == 24)
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0,
                                  GL_BGR, GL_UNSIGNED_BYTE, bits);
    else if (colours == 32)
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height,
                                  0, GL_BGRA, GL_UNSIGNED_BYTE, bits);
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
int LoadSceneFromFile(std::string file_name, GLuint& vao,
                      std::vector<GLfloat>& mesh_vertices_count,
                      std::vector<GLfloat>& mesh_starting_vertex_index,
                      std::vector<GLuint>& textures)
{
    const aiScene* scene = aiImportFile(file_name.c_str(), aiProcess_Triangulate);
    if (!scene)
    {
        std::cout << "Mesh not found." << std::endl;
        return -1;
    }

    int total_vertices_count = 0;

    std::vector<GLfloat> buffer_vbo_data;

    for (unsigned int i = 0; i != scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i];

        int mesh_vertices = 0;

        for (unsigned int j = 0; j != mesh->mNumFaces; j++)
        {
            const aiFace* face = &mesh->mFaces[j];

            for (int k = 0; k != 3; k++)
            {
                aiVector3D vertex_position{ 0, 0, 0 };
                aiVector3D vertex_normal{ 0, 0, 0 };
                aiVector3D vertex_texture_coord{ 0, 0, 0 };

                if (mesh->HasPositions())
                    vertex_position = mesh->mVertices[face->mIndices[k]];

                if (mesh->HasNormals())
                    vertex_normal = mesh->mNormals[face->mIndices[k]];

                if (mesh->HasTextureCoords(0))
                    vertex_texture_coord = mesh->mTextureCoords[0][face->mIndices[k]];

                buffer_vbo_data.push_back(vertex_position.x);
                buffer_vbo_data.push_back(vertex_position.y);
                buffer_vbo_data.push_back(vertex_position.z);

                buffer_vbo_data.push_back(vertex_normal.x);
                buffer_vbo_data.push_back(vertex_normal.y);
                buffer_vbo_data.push_back(vertex_normal.z);

                buffer_vbo_data.push_back(vertex_texture_coord.x);
                buffer_vbo_data.push_back(vertex_texture_coord.y);

                mesh_vertices++;
            }
        }

        mesh_vertices_count.push_back(mesh_vertices);
        mesh_starting_vertex_index.push_back(total_vertices_count);
        total_vertices_count += mesh_vertices;

        if (scene->mNumMaterials != 0)
        {
            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            aiString texture_path;

            GLuint tex = 0;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) ==
                AI_SUCCESS)
            {
                unsigned int found_pos = file_name.find_last_of("/\\");
                std::string path = file_name.substr(0, found_pos);
                std::string name(texture_path.C_Str());
                if (name[0] == '/')
                    name.erase(0, 1);

                std::string file_path = path + "/" + name;


                if (LoadTexture(file_path, tex))
                    std::cout << "Texture " << file_path << " not found." <<
                                 std::endl;
                else
                    std::cout << "Texture " << file_path << " loaded." <<
                                 std::endl;
            }

            textures.push_back(tex);
        }
    }

    GLuint vbo_buffer = 0;
    glGenBuffers(1, &vbo_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_buffer);
    glBufferData(GL_ARRAY_BUFFER, buffer_vbo_data.size() * sizeof(GLfloat),
                 buffer_vbo_data.data(), GL_STATIC_DRAW);

    int single_vertex_size = 2 * 3 * sizeof(GLfloat) + 2 * sizeof(GLfloat);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, single_vertex_size, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, single_vertex_size,
                          reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, single_vertex_size,
                          reinterpret_cast<void*>(2 * 3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    return 0;
}
//******************************************************************************
int main()
{
    int result = CreateWindow(640, 480, "GL Window", 4, false);
    if (result)
        return -1;

    GLuint vao1 = 0;
    std::vector<GLfloat> vertices_count;
    std::vector<GLfloat> starting_vertex;
    std::vector<GLuint> textures;

    LoadSceneFromFile("farm/Farmhouse OBJ.obj", vao1, vertices_count,
                      starting_vertex, textures);

    // Zaladowanie shaderow i pobranie lokalizacji zmiennych uniform
    GLuint shaders = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    ActivateShaderProgram(shaders);

    GLint texture_slot = glGetUniformLocation(shaders, "basic_texture");
    glUniform1i(texture_slot, 0);

    GLint view_uniform = glGetUniformLocation(shaders, "view_matrix");
    if(view_uniform == -1)
        std::cout << "Variable 'view_matrix' not found." << std::endl;

    GLint perspective_uniform = glGetUniformLocation(shaders, "perspective_matrix");
    if(perspective_uniform == -1)
        std::cout << "Variable 'perspective_matrix' not found." << std::endl;

    // Obliczenie pozycji poczatkowej kamery
    camera_up = glm::cross(camera_right, camera_direction);
    view_matrix = glm::lookAt(camera_position, camera_position +
                              camera_direction, camera_up);

    // Ustawienie perspektywy
    aspect = float(window_width) / float(window_height);

    perspective = glm::perspective(FOV, aspect, P1, P2);

    // Wyslanie perspektywy i pozycji kamery do programu shadera
    glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(perspective_uniform, 1, GL_FALSE,
                       glm::value_ptr(perspective));

    // Wlaczenie depth testing i face culling
    EnableDepthTesting();
    //EnableFaceCulling();

    while (RenderingEnabled())
    {
        static double fps = 0;
        FPSCounter(fps);
        std::string title = "GL Window @ FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window, title.c_str());

        previous_time = actual_time;
        actual_time = glfwGetTime();

        ClearColor(0.5, 0.5, 0.5);

        // Wyslanie perspektywy i kamery do programu shadera
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(perspective_uniform, 1, GL_FALSE, glm::value_ptr(perspective));

        // Rysowanie
        glBindVertexArray(vao1);
        for (unsigned int i = 0; i < starting_vertex.size(); i++)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glUniform1i(texture_slot, 0);
            glDrawArrays(GL_TRIANGLES, starting_vertex[i], vertices_count[i]);
        }

        ProcessWindowEvents();
    }

    Terminate();

    return 0;
}
