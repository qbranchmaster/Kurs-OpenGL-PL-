//******************************************************************************
// Kurs OpenGL - krok po kroku
// http://kurs-opengl.pl
// Sebastian Tabaka
//******************************************************************************
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define GLFW_DLL
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <FreeImage.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
//******************************************************************************
GLFWwindow *window_handle = nullptr;
int window_width = 0;
int window_height = 0;

enum class ShaderType
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
};

double actual_time;
double previous_time;

float P1 = 0.1f;
float P2 = 200.0f;
float FOV = 1.15f;
float aspect;
float camera_horizontal_angle = 0;
float camera_vertical_angle = 0;
glm::vec3 camera_position(100.0, 5.0, 50.0);
glm::vec3 camera_direction(1.0, 0.0, 0.0);
glm::vec3 camera_right(1.0, 0.0, 0.0);
glm::vec3 camera_up;
glm::mat4 view_matrix;
glm::mat4 perspective;

struct Mesh
{
    GLuint handle = 0;
    GLuint diffuse_texture = 0;
    GLuint normalmap_texture = 0;
    unsigned int vertices_count = 0;
};

struct Texture
{
    BYTE *bits;
    FIBITMAP *image_ptr;
    int width;
    int height;
};

typedef std::vector<Mesh*> MeshHandle;
//******************************************************************************
double getTimeDelta()
{
    return (actual_time - previous_time);
}
//******************************************************************************
void updateTimer()
{
    previous_time = actual_time;
    actual_time = glfwGetTime();
}
//******************************************************************************
void lockCursor(GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(window, window_width / 2.0, window_height / 2.0);
}
//******************************************************************************
void closeWindow(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, GL_TRUE);
}
//******************************************************************************
void takeScreenshot(std::string file_name)
{
    unsigned char *screen_buffer = new unsigned char[window_width *
        window_height * 3];

    glReadPixels(0, 0, window_width, window_height, GL_BGR, GL_UNSIGNED_BYTE,
                 screen_buffer);

    FIBITMAP *image = FreeImage_ConvertFromRawBits(screen_buffer, window_width,
                                                   window_height, 3 * window_width,
                                                   24, 0xFF0000, 0x00FF00,
                                                   0x0000FF, false);

    FreeImage_Save(FIF_PNG, image, file_name.c_str());

    delete screen_buffer;

    std::cout << "Screenshot \"" << file_name << "\" saved." << std::endl;
}
//******************************************************************************
void setCameraAngles(float horizontal, float vertical)
{
    camera_direction = glm::vec3(cos(vertical) * sin(horizontal),
                                 sin(vertical),
                                 cos(vertical) * cos(horizontal));

    camera_right = glm::vec3(-cos(horizontal), 0, sin(horizontal));

    camera_up = glm::cross(camera_right, camera_direction);
    view_matrix = glm::lookAt(camera_position, camera_position +
                              camera_direction, camera_up);
}
//******************************************************************************
void recalculateCamera()
{
    camera_up = glm::cross(camera_right, camera_direction);
    view_matrix = glm::lookAt(camera_position, camera_position +
                              camera_direction, camera_up);

    view_matrix = glm::lookAt(camera_position, camera_position +
                              camera_direction, camera_up);

    perspective = glm::perspective(FOV, aspect, P1, P2);
}
//******************************************************************************
void cursorPositionCallback(GLFWwindow *window, double x_cursor_pos,
                            double y_cursor_pos)
{
    lockCursor(window);

    float mouse_speed = 0.5f;
    float time_delta = static_cast<float>(getTimeDelta());

    double x_delta = x_cursor_pos - window_width / 2.0;
    double y_delta = y_cursor_pos - window_height / 2.0;

    camera_horizontal_angle += mouse_speed * static_cast<float>(time_delta) *
        static_cast<float>(-x_delta);
    camera_vertical_angle += mouse_speed * static_cast<float>(time_delta) *
        static_cast<float>(-y_delta);

    if (camera_horizontal_angle > 2 * glm::pi<float>() ||
        camera_horizontal_angle < -2 * glm::pi<float>())
        camera_horizontal_angle = 0.0f;

    if (camera_vertical_angle >= glm::half_pi<float>())
        camera_vertical_angle = glm::half_pi<float>();
    if (camera_vertical_angle <= -glm::half_pi<float>())
        camera_vertical_angle = -glm::half_pi<float>();

    setCameraAngles(camera_horizontal_angle, camera_vertical_angle);
}
//******************************************************************************
void KeyCallback(GLFWwindow *window, int key, int /*scancode*/, int action,
                 int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        closeWindow(window);
        return;
    }

    if (key == GLFW_KEY_T && action == GLFW_RELEASE)
        takeScreenshot("test.png");

    float move_speed = 100.0f;
    float time_delta = static_cast<float>(getTimeDelta());

    if (key == GLFW_KEY_W)
        camera_position += camera_direction * time_delta * move_speed;
    if (key == GLFW_KEY_S)
        camera_position -= camera_direction * time_delta * move_speed;
    if (key == GLFW_KEY_A)
        camera_position -= camera_right * time_delta * move_speed;
    if (key == GLFW_KEY_D)
        camera_position += camera_right * time_delta * move_speed;

    recalculateCamera();
}
//******************************************************************************
void setCursorPos(double x, double y)
{
    glfwSetCursorPos(window_handle, x, y);
}
//******************************************************************************
void windowSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    window_width = width;
    window_height = height;

    aspect = float(window_width) / float(window_height);
    recalculateCamera();
}
//******************************************************************************
int createWindow(int width, int height, std::string name, int samples,
                 bool fullscreen)
{
    if (name.empty())
        name = "Unnamed Window";

    if (width == 0 || height == 0)
    {
        std::cout << "Wrong window's \"" << name << "\" size." << std::endl;
        return -1;
    }

    if (!glfwInit())
    {
        std::cout << "GLFW initialization error." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, samples);

    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

    window_handle = glfwCreateWindow(width, height, name.c_str(),
                                     fullscreen ? glfwGetPrimaryMonitor() :
                                     nullptr, nullptr);

    if (!window_handle)
    {
        std::cout << "Error creating window \"" << name << "\"." << std::endl;
        glfwTerminate();
        return -1;
    }

    window_width = width;
    window_height = height;

    glfwMakeContextCurrent(window_handle);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW initialization error." << std::endl;
        return -1;
    }

    setCursorPos(window_width / 2.0, window_height / 2.0);
    glfwSetKeyCallback(window_handle, KeyCallback);
    glfwSetCursorPosCallback(window_handle, cursorPositionCallback);
    glfwSetWindowSizeCallback(window_handle, windowSizeCallback);

    return 0;
}
//******************************************************************************
int loadShaderCode(std::string file_name, std::string &shader_code)
{
    std::ifstream shader_file(file_name, std::ios::in);
    if (shader_file.is_open())
    {
        std::string line;
        while (std::getline(shader_file, line))
            shader_code += line + "\n";

        shader_file.close();
    }
    else
        return -1;

    return 0;
}
//******************************************************************************
int checkShaderCompileStatus(GLuint shader_handle)
{
    int shader_status = -1;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &shader_status);
    if (shader_status != GL_TRUE)
        return -1;

    return 0;
}
//******************************************************************************
int compileShader(GLuint shader_handle)
{
    glCompileShader(shader_handle);

    if (checkShaderCompileStatus(shader_handle))
        return -1;

    return 0;
}
//******************************************************************************
std::string getShaderCompileMsg(GLuint shader_handle)
{
    const unsigned int buffer_size = 2048;
    int length = 0;
    char log_text[buffer_size];
    glGetShaderInfoLog(shader_handle, buffer_size, &length, log_text);
    std::string compile_msg = std::string(log_text);

    return compile_msg;
}
//******************************************************************************
int loadShader(GLuint &shader_handle, std::string file_name,
               ShaderType shader_type)
{
    std::string shader_data;
    if (loadShaderCode(file_name, shader_data))
    {
        std::cout << "Error opening shader file \"" + file_name + "\"." <<
            std::endl;
        return -1;
    }

    std::cout << "Shader file \"" << file_name << "\" loaded." << std::endl;

    if (shader_type == ShaderType::VERTEX_SHADER)
        shader_handle = glCreateShader(GL_VERTEX_SHADER);
    else if (shader_type == ShaderType::FRAGMENT_SHADER)
        shader_handle = glCreateShader(GL_FRAGMENT_SHADER);

    const char *shader_code = shader_data.c_str();
    glShaderSource(shader_handle, 1, &shader_code, nullptr);

    if (compileShader(shader_handle))
    {
        std::cout << "Shader \"" << file_name << "\" compile error." <<
            std::endl;
        std::cout << "Shader compile log:" << std::endl;
        std::cout << getShaderCompileMsg(shader_handle) << std::endl;
        return -1;
    }

    std::cout << "Shader file \"" << file_name << "\" compile success." <<
        std::endl;

    return 0;
}
//******************************************************************************
int checkShaderProgramLinkStatus(GLuint shader_program)
{
    int link_status = -1;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE)
        return -1;

    return 0;
}
//******************************************************************************
int linkShaderProgram(GLuint &shader_program, GLuint vertex_shader_handle,
                      GLuint fragment_shader_handle)
{
    glAttachShader(shader_program, vertex_shader_handle);
    glAttachShader(shader_program, fragment_shader_handle);

    glLinkProgram(shader_program);

    if (checkShaderProgramLinkStatus(shader_program))
    {
        std::cout << "Shader program link error." << std::endl;
        return -1;
    }

    std::cout << "Shader program link success." << std::endl;

    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);

    return 0;
}
//******************************************************************************
int createShaderProgram(GLuint &handle, std::string vertex_shader_file,
                        std::string fragment_shader_file)
{
    GLuint vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);

    if (loadShader(vertex_shader_handle, vertex_shader_file,
                   ShaderType::VERTEX_SHADER))
        return -1;

    if (loadShader(fragment_shader_handle, fragment_shader_file,
                   ShaderType::FRAGMENT_SHADER))
        return -1;

    handle = glCreateProgram();

    if (linkShaderProgram(handle, vertex_shader_handle, fragment_shader_handle))
        return -1;

    std::cout << "Shader program created successfully." << std::endl;

    return 0;
}
//******************************************************************************
void enableDepthTesting()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}
//******************************************************************************
void enableFaceCulling()
{
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}
//******************************************************************************
void processWindowEvents()
{
    glfwSwapBuffers(window_handle);
    glfwPollEvents();
}
//******************************************************************************
void terminate()
{
    glfwDestroyWindow(window_handle);
    glfwTerminate();
}
//******************************************************************************
bool renderingEnabled()
{
    if (glfwWindowShouldClose(window_handle))
        return false;

    return true;
}
//******************************************************************************
void clearColor(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window_width, window_height);
}
//******************************************************************************
void activateShaderProgram(GLuint shader_program)
{
    glUseProgram(shader_program);
}
//******************************************************************************
int loadTexture(std::string file_name, Texture &texture)
{
    FREE_IMAGE_FORMAT image_format = FIF_UNKNOWN;
    FIBITMAP *image_ptr = nullptr;
    BYTE *bits = nullptr;

    image_format = FreeImage_GetFileType(file_name.c_str(), 0);
    if (image_format == FIF_UNKNOWN)
        image_format = FreeImage_GetFIFFromFilename(file_name.c_str());

    if (image_format == FIF_UNKNOWN)
    {
        std::cout << "Texture \"" << file_name << "\" has unknown file format." <<
            std::endl;
        return -1;
    }

    if (FreeImage_FIFSupportsReading(image_format))
        image_ptr = FreeImage_Load(image_format, file_name.c_str());

    if (!image_ptr)
    {
        std::cout << "Unable to load texture \"" << file_name << "\"." << std::endl;
        return -1;
    }

    bits = FreeImage_GetBits(image_ptr);

    unsigned int image_width = 0;
    unsigned int image_height = 0;
    image_width = FreeImage_GetWidth(image_ptr);
    image_height = FreeImage_GetHeight(image_ptr);

    if ((bits == 0) || (image_width == 0) || (image_height == 0))
    {
        std::cout << "Texture \"" << file_name << "\" format error." << std::endl;
        return -1;
    }

    std::cout << "Texture \"" << file_name << "\" loaded." << std::endl;

    texture.bits = bits;
    texture.image_ptr = image_ptr;
    texture.width = image_width;
    texture.height = image_height;

    return 0;
}
//******************************************************************************
int loadTexture2D(GLuint& texture_handle, Texture texture)
{
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    unsigned int colours = FreeImage_GetBPP(texture.image_ptr);
    if (colours == 24)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height,
                     0, GL_BGR, GL_UNSIGNED_BYTE, texture.bits);
    else if (colours == 32)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height,
                     0, GL_BGRA, GL_UNSIGNED_BYTE, texture.bits);

    glGenerateMipmap(GL_TEXTURE_2D);

    GLfloat anisotropy_factor = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy_factor);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy_factor);

    glBindTexture(GL_TEXTURE_2D, 0);

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
int loadSceneFromFile(std::string file_name, std::vector<Mesh*>& mesh_handle)
{
    const aiScene* scene = aiImportFile(file_name.c_str(),
                                        aiProcessPreset_TargetRealtime_Fast);
    if (!scene)
    {
        std::cout << "Mesh file \"" << file_name << "\" not found." << std::endl;
        return -1;
    }

    std::vector<Mesh*> complete_mesh;

    for (unsigned int m = 0; m != scene->mNumMeshes; m++)
    {
        aiMesh *mesh = scene->mMeshes[m];

        Mesh *mesh_entity = new Mesh();
        std::vector<GLfloat> position_container;
        std::vector<GLfloat> normal_vector_container;
        std::vector<GLfloat> texture_coord_container;
        std::vector<GLfloat> tangent_container;
        std::vector<GLfloat> bitangent_container;

        for (unsigned int f = 0; f != mesh->mNumFaces; f++)
        {
            const aiFace *face = &mesh->mFaces[f];

            for (unsigned int v = 0; v != 3; v++)
            {
                aiVector3D position{ 0, 0, 0 };
                aiVector3D normal_vector{ 0, 0, 0 };
                aiVector3D texture_coords{ 0, 0, 0 };
                aiVector3D tangent{ 0, 0, 0 };
                aiVector3D bitangent{ 0, 0, 0 };

                if (mesh->HasPositions())
                    position = mesh->mVertices[face->mIndices[v]];

                if (mesh->HasNormals())
                    normal_vector = mesh->mNormals[face->mIndices[v]];

                if (mesh->HasTextureCoords(0))
                    texture_coords = mesh->mTextureCoords[0][face->mIndices[v]];

                if (mesh->HasTangentsAndBitangents())
                {
                    tangent = mesh->mTangents[face->mIndices[v]];
                    bitangent = mesh->mBitangents[face->mIndices[v]];
                }

                position_container.push_back(position.x);
                position_container.push_back(position.y);
                position_container.push_back(position.z);

                normal_vector_container.push_back(normal_vector.x);
                normal_vector_container.push_back(normal_vector.y);
                normal_vector_container.push_back(normal_vector.z);

                texture_coord_container.push_back(texture_coords.x);
                texture_coord_container.push_back(texture_coords.y);

                glm::vec3 n(normal_vector.x, normal_vector.y,
                            normal_vector.z);
                glm::vec3 t(tangent.x, tangent.y, tangent.z);
                glm::vec3 b(bitangent.x, bitangent.y, bitangent.z);

                glm::vec3 tangent_corrected = glm::normalize(t - n * glm::dot(n, t));

                float det = glm::dot(glm::cross(n, t), b);
                if (det < 0.0f)
                    det = -1.0f;
                else
                    det = 1.0f;

                glm::vec3 bitangent_corrected = glm::cross(n, tangent_corrected) * det;

                tangent_container.push_back(tangent_corrected.x);
                tangent_container.push_back(tangent_corrected.y);
                tangent_container.push_back(tangent_corrected.z);

                bitangent_container.push_back(bitangent_corrected.x);
                bitangent_container.push_back(bitangent_corrected.y);
                bitangent_container.push_back(bitangent_corrected.z);
            }
        }

        GLuint position_vbo = 0;
        glGenBuffers(1, &position_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
        glBufferData(GL_ARRAY_BUFFER, position_container.size() * sizeof(GLfloat),
                     position_container.data(), GL_STATIC_DRAW);

        GLuint normal_vector_vbo = 0;
        glGenBuffers(1, &normal_vector_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, normal_vector_vbo);
        glBufferData(GL_ARRAY_BUFFER, normal_vector_container.size() * sizeof(GLfloat),
                     normal_vector_container.data(), GL_STATIC_DRAW);

        GLuint texture_coord_vbo = 0;
        glGenBuffers(1, &texture_coord_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, texture_coord_vbo);
        glBufferData(GL_ARRAY_BUFFER, texture_coord_container.size() * sizeof(GLfloat),
                     texture_coord_container.data(), GL_STATIC_DRAW);

        GLuint tangent_vbo = 0;
        glGenBuffers(1, &tangent_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, tangent_vbo);
        glBufferData(GL_ARRAY_BUFFER, tangent_container.size() * sizeof(GLfloat),
                     tangent_container.data(), GL_STATIC_DRAW);

        GLuint bitangent_vbo = 0;
        glGenBuffers(1, &bitangent_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, bitangent_vbo);
        glBufferData(GL_ARRAY_BUFFER, bitangent_container.size() * sizeof(GLfloat),
                     bitangent_container.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &mesh_entity->handle);
        glBindVertexArray(mesh_entity->handle);

        glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, normal_vector_vbo);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, texture_coord_vbo);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, tangent_vbo);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, bitangent_vbo);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);

        mesh_entity->vertices_count = position_container.size();

        if (scene->mNumMaterials != 0)
        {
            const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            aiString texture_path;

            GLuint texture_diffuse = 0;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) ==
                AI_SUCCESS)
            {
                unsigned int found_pos = file_name.find_last_of("/\\");
                std::string path = file_name.substr(0, found_pos);
                std::string name(texture_path.C_Str());
                if (name[0] == '/')
                    name.erase(0, 1);

                std::string file_path = path + "/" + name;

                Texture tex;
                if (loadTexture(file_path, tex))
                    std::cout << "Texture \"" << file_path << "\" not found." <<
                    std::endl;
                else
                {
                    loadTexture2D(texture_diffuse, tex);
                    std::cout << "Texture \"" << file_path << "\" loaded." <<
                        std::endl;
                }
            }

            GLuint texture_normalmap = 0;
            if (material->GetTexture(aiTextureType_HEIGHT, 0, &texture_path) ==
                AI_SUCCESS)
            {
                unsigned int found_pos = file_name.find_last_of("/\\");
                std::string path = file_name.substr(0, found_pos);
                std::string name(texture_path.C_Str());
                if (name[0] == '/')
                    name.erase(0, 1);

                std::string file_path = path + "/" + name;

                Texture tex;
                if (loadTexture(file_path, tex))
                    std::cout << "Texture \"" << file_path << "\" not found." <<
                    std::endl;
                else
                {
                    loadTexture2D(texture_normalmap, tex);
                    std::cout << "Texture \"" << file_path << "\" loaded." <<
                        std::endl;
                }
            }

            mesh_entity->diffuse_texture = texture_diffuse;
            mesh_entity->normalmap_texture = texture_normalmap;
        }

        complete_mesh.push_back(mesh_entity);
    }

    mesh_handle = complete_mesh;

    return 0;
}
//******************************************************************************
int findUniform(GLuint shader_program, std::string uniform_name)
{
    GLint result = glGetUniformLocation(shader_program, uniform_name.c_str());

    if (result == -1)
        std::cout << "Uniform \"" << uniform_name << "\" not found." << std::endl;

    return result;
}
//******************************************************************************
void setUniform(GLint uniform_handle, GLint value)
{
    glUniform1iv(uniform_handle, 1, &value);
}
//******************************************************************************
void setUniform(GLint uniform_handle, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(uniform_handle, 1, GL_FALSE, glm::value_ptr(matrix));
}
//******************************************************************************
void drawMesh(const MeshHandle& mesh)
{
    for (const auto &it : mesh)
    {
        glBindVertexArray(it->handle);
        // Diffuse texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, it->diffuse_texture);
        // Normalmap texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, it->normalmap_texture);

        glDrawArrays(GL_TRIANGLES, 0, it->vertices_count);
    }
}
//******************************************************************************
void loadSkybox(std::string front, std::string back, std::string left,
                std::string right, std::string up, std::string down,
                GLuint &texture_handle)
{
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_handle);

    std::string textures[] = { right, left, down, up, back, front };

    for (int i = 0; i < 6; i++)
    {
        Texture texture;
        loadTexture(textures[i], texture);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, texture.width,
                     texture.height, 0, GL_BGR, GL_UNSIGNED_BYTE, texture.bits);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
//******************************************************************************
float calculateHeight(Texture *texture, int x, int y)
{
    const float MAX_HEIGHT = 20.0f;

    RGBQUAD colour;
    FreeImage_GetPixelColor(texture->image_ptr, x, y, &colour);

    float height = colour.rgbRed * colour.rgbGreen * colour.rgbBlue;

    if (height > 256 * 256 * 256 / 2)
        height = height;
    else
        height = -height;

    height /= 256 * 256 * 256 / 2;
    height *= MAX_HEIGHT;

    return height;
}
//******************************************************************************
glm::vec3 calculateNormal(Texture *texture, int x, int z)
{
    float hL = calculateHeight(texture, x - 1, z);
    float hR = calculateHeight(texture, x + 1, z);
    float hD = calculateHeight(texture, x, z - 1);
    float hU = calculateHeight(texture, x, z + 1);

    glm::vec3 new_normal_vector = glm::vec3(hL - hR, 2.0, hD - hU);
    glm::normalize(new_normal_vector);
    return new_normal_vector;
}
//******************************************************************************
GLuint generateTerrain(std::string height_map, unsigned short cell_size, int &elements, int &cells)
{
    Texture height_map_tex;
    loadTexture(height_map, height_map_tex);

    int vertices_count = height_map_tex.width;

    std::vector<float> positions;
    std::vector<float> normal_vectors;
    std::vector<float> texture_coords;

    std::vector<int> indices;        

    for (int w = 0; w < vertices_count; w++)
    {
        for (int k = 0; k < vertices_count; k++)
        {
            float vertex_x = k * cell_size;
            float vertex_y = calculateHeight(&height_map_tex, k, w);
            float vertex_z = w * cell_size;

            positions.push_back(vertex_x);
            positions.push_back(vertex_y);
            positions.push_back(vertex_z);

            auto normal = calculateNormal(&height_map_tex, k, w);

            float vector_x = normal.x;
            float vector_y = normal.y;
            float vector_z = normal.z;

            normal_vectors.push_back(vector_x);
            normal_vectors.push_back(vector_y);
            normal_vectors.push_back(vector_z);

            float s = (1.0 / (vertices_count - 1)) * k;
            float t = (1.0 / (vertices_count - 1)) * w;

            texture_coords.push_back(s);
            texture_coords.push_back(t);
        }
    }

    for (int w = 0; w < vertices_count - 1; w++)
    {
        for (int k = 0; k < vertices_count - 1; k++)
        {
            int index_1 = w * vertices_count + k;
            int index_2 = index_1 + 1;
            int index_3 = (w + 1) * vertices_count + k;
            int index_4 = index_3 + 1;

            indices.push_back(index_1);
            indices.push_back(index_3);
            indices.push_back(index_2);

            indices.push_back(index_2);
            indices.push_back(index_3);
            indices.push_back(index_4);
        }
    }

    GLuint position_vbo = 0;
    glGenBuffers(1, &position_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(),
                 GL_STATIC_DRAW);

    GLuint normal_vector_vbo = 0;
    glGenBuffers(1, &normal_vector_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, normal_vector_vbo);
    glBufferData(GL_ARRAY_BUFFER, normal_vectors.size() * sizeof(GLfloat), normal_vectors.data(),
                 GL_STATIC_DRAW);

    GLuint texture_coords_vbo = 0;
    glGenBuffers(1, &texture_coords_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, texture_coords_vbo);
    glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(GLfloat), texture_coords.data(),
                 GL_STATIC_DRAW);

    GLuint indices_vbo = 0;
    glGenBuffers(1, &indices_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(),
                 GL_STATIC_DRAW);

    GLuint handle = 0;
    glGenVertexArrays(1, &handle);
    glBindVertexArray(handle);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normal_vector_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, texture_coords_vbo);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

    elements = indices.size();
    cells = vertices_count - 1;
    return handle;
}
//******************************************************************************
int main()
{
    int result = createWindow(800, 600, "GL Window", 4, false);
    if (result)
        return -1;

    GLuint shader_program = 0;
    if (createShaderProgram(shader_program, "vertex_shader.glsl",
                            "fragment_shader.glsl"))
        return -1;

    GLuint skybox_shader = 0;
    if (createShaderProgram(skybox_shader, "vertex_shader_skybox.glsl",
                            "fragment_shader_skybox.glsl"))
        return -1;

    GLuint terrain_shader = 0;
    if (createShaderProgram(terrain_shader, "vertex_shader_terrain.glsl",
                            "fragment_shader_terrain.glsl"))
        return -1;

    GLint texture_slot = findUniform(shader_program, "basic_texture");
    GLint view_uniform = findUniform(shader_program, "view_matrix");
    GLint perspective_uniform = findUniform(shader_program, "perspective_matrix");
    GLint model_uniform = findUniform(shader_program, "model_matrix");

    GLint view_uniform_sky = findUniform(skybox_shader, "view_matrix");
    GLint perspective_uniform_sky = findUniform(skybox_shader, "perspective_matrix");

    GLint view_uniform_terrain = findUniform(terrain_shader, "view_matrix");
    GLint perspective_uniform_terrain = findUniform(terrain_shader, "perspective_matrix");
    GLint texture_slot_terrain = findUniform(terrain_shader, "basic_texture");
    GLint cells_terrain = findUniform(terrain_shader, "cells");

    aspect = float(window_width) / float(window_height);
    recalculateCamera();

    // Load terrain
    int elements = 0;
    int cells = 0;
    GLuint terrain_handle = generateTerrain("height_map.png", 1, elements, cells);
    Texture grass_texture;
    unsigned int grass_tex = 0;
    loadTexture("Grass.jpg", grass_texture);
    loadTexture2D(grass_tex, grass_texture);

    // Skybox
    GLfloat skybox[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f, 1.0f,
        1.0f,  1.0f, 1.0f,
        1.0f,  1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f,  1.0f,
        1.0f, 1.0f,  1.0f,
        -1.0f, 1.0f,  1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    GLuint skybox_vbo = 0;
    glGenBuffers(1, &skybox_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox), &skybox, GL_STATIC_DRAW);

    GLuint skybox_vao = 0;
    glGenVertexArrays(1, &skybox_vao);
    glBindVertexArray(skybox_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint skybox_texture = 0;
    loadSkybox("hills_ft.tga", "hills_bk.tga", "hills_lf.tga", "hills_rt.tga",
               "hills_up.tga", "hills_dn.tga", skybox_texture);

    enableFaceCulling();

    while (renderingEnabled())
    {
        static double fps = 0;
        FPSCounter(fps);
        std::string title = "GL Window @ FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window_handle, title.c_str());

        updateTimer();

        clearColor(0.5, 0.5, 0.5);

        activateShaderProgram(skybox_shader);
        glm::mat4 view_static = view_matrix;
        glm::vec3 pos(0.0, 0.0, 0.0);
        view_static = glm::lookAt(pos, pos + camera_direction, camera_up);
        setUniform(view_uniform_sky, view_static);
        setUniform(perspective_uniform_sky, perspective);

        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        glBindVertexArray(skybox_vao);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDisableVertexAttribArray(0);
        glEnable(GL_DEPTH_TEST);

        // Draw terrain
        activateShaderProgram(terrain_shader);
        setUniform(view_uniform_terrain, view_matrix);
        setUniform(perspective_uniform_terrain, perspective);
        setUniform(texture_slot_terrain, 0);
        setUniform(cells_terrain, cells);
        glBindVertexArray(terrain_handle);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass_tex);
        glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glBindVertexArray(0);

        processWindowEvents();
    }

    terminate();
    return 0;
}