//******************************************************************************
// Kurs OpenGL - krok po kroku
// http://kurs-opengl.pl
// Sebastian Tabaka
//******************************************************************************
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
#include <map>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
//******************************************************************************
// Declarations
GLFWwindow *window_handle = nullptr;
int window_height = 0;
int window_width = 0;

enum class ShaderType
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
};

double actual_time = 0;
double previous_time = 0;

float P1 = 0.1f;
float P2 = 200.0f;
float FOV = 1.15f;
float aspect;
float camera_horizontal_angle = 0;
float camera_vertical_angle = 0;

glm::vec3 camera_position(0.0, 15.0, -50.0);
glm::vec3 camera_direction(0.0, 0.0, 1.0);
glm::vec3 camera_right(-1.0, 0.0, 0.0);
glm::vec3 camera_up;
glm::mat4 view_matrix;
glm::mat4 projection_matrix;

struct Mesh
{
    GLuint handle = 0;
    GLuint diffuse_texture = 0;
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

bool renderingEnabled();
double getTimeDelta();
int checkShaderCompileStatus(GLuint shader_handle);
int checkShaderProgramLinkStatus(GLuint shader_program);
int compileShader(GLuint shader_handle);
int createShaderProgram(GLuint &handle, std::string vertex_shader_file,
                        std::string fragment_shader_file);
int createWindow(int width, int height, std::string name, int samples, bool fullscreen);
int findUniform(GLuint shader_program, std::string uniform_name);
int linkShaderProgram(GLuint &shader_program, GLuint vertex_shader_handle,
                      GLuint fragment_shader_handle);
int loadSceneFromFile(std::string file_name, std::vector<Mesh*>& mesh_handle);
int loadShader(GLuint &shader_handle, std::string file_name,
               ShaderType shader_type);
int loadShaderCode(std::string file_name, std::string &shader_code);
int loadTexture(std::string file_name, Texture &texture);
int loadTexture2D(GLuint& texture_handle, Texture texture);
std::string getShaderCompileMsg(GLuint shader_handle);
void activateShaderProgram(GLuint shader_program);
void clearColor(float r, float g, float b);
void closeWindow(GLFWwindow *window);
void drawMesh(const MeshHandle& mesh);
void enableDepthTesting(bool state);
void enableFaceCulling(bool state);
void FPSCounter(double& fps);
void freeTextureData(Texture &texture);
void loadTextureSkybox(std::string front, std::string back, std::string left, std::string right,
                       std::string up, std::string down, GLuint &texture_handle);
void pollKeyboad();
void pollMouse();
void processWindowEvents();
void recalculateCamera();
void setCameraAngles(float horizontal, float vertical);
void setCursorPos(double x, double y);
void setUniform(GLint uniform_handle, GLint value);
void setUniform(GLint uniform_handle, const glm::mat4 &matrix);
void setUniform(GLint uniform_handle, const glm::vec3 &vector);
void terminate();
void updateTimer();
void windowSizeCallback(GLFWwindow *, int width, int height);

class FreeTypeFontRenderer
{
public:
    FreeTypeFontRenderer(std::string font_name, unsigned int size)
    {
        if (FT_Init_FreeType(&ft_library_))
        {
            std::cout << "FreeType initialization error." << std::endl;
            return;
        }

        std::cout << "FreeType initialization success." << std::endl;
        std::cout << "Loading font: " << font_name << " ..." << std::endl;

        int result = FT_New_Face(ft_library_, font_name.c_str(), 0, &font_face_);
        if (result == FT_Err_Unknown_File_Format)
        {
            std::cout << "Unknown font file format." << std::endl;
            return;
        }
        else if (result)
        {
            std::cout << "Font loading error. Does font exist?" << std::endl;
            return;
        }

        std::cout << "Font " << font_name << " loaded." << std::endl;

        FT_Set_Pixel_Sizes(font_face_, 0, size);

        glGenTextures(1, &font_texture_handle_);

        glBindTexture(GL_TEXTURE_2D, font_texture_handle_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenVertexArrays(1, &handle_);

        glGenBuffers(1, &vertices_vbo_);
        glGenBuffers(1, &texture_coords_vbo_);

        float texture_coords[] = {
            0, 0,
            0, 1,
            1, 0,
            1, 0,
            0, 1,
            1, 1
        };

        glBindVertexArray(handle_);
        glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo_);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, texture_coords_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), texture_coords, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    ~FreeTypeFontRenderer()
    {
        glDeleteTextures(1, &font_texture_handle_);

        glDeleteBuffers(1, &vertices_vbo_);
        glDeleteBuffers(1, &texture_coords_vbo_);

        glDeleteVertexArrays(1, &handle_);
    }

    float calculateScreenCoordX(float x)
    {
        return (x / static_cast<float>(viewport_width_) * 2 - 1);
    }

    float calculateScreenCoordY(float y)
    {
        return (1 - y / static_cast<float>(viewport_height_) * 2);
    }

    void renderText(std::wstring text, int x, int y)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font_texture_handle_);

        glBindVertexArray(handle_);

        int cursor_pos_x = x;
        int cursor_pos_y = y;

        std::vector<GLfloat> vertices_buffer;

        for (const auto &character : text)
        {
            if (FT_Load_Char(font_face_, character, FT_LOAD_RENDER))
                continue;

            if (character == '\n')
            {
                cursor_pos_x = x;
                cursor_pos_y += font_face_->glyph->bitmap.rows + font_face_->glyph->bitmap_top;
                continue;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font_face_->glyph->bitmap.width,
                         font_face_->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                         font_face_->glyph->bitmap.buffer);

            std::vector<glm::vec3> vertices(4);

            vertices[0] = glm::vec3{
                cursor_pos_x + font_face_->glyph->bitmap_left,
                cursor_pos_y - font_face_->glyph->bitmap_top,
                0.0};

            vertices[1] = glm::vec3{
                vertices[0].x,
                vertices[0].y + font_face_->glyph->bitmap.rows,
                0.0};

            vertices[2] = glm::vec3{
                vertices[0].x + font_face_->glyph->bitmap.width,
                vertices[0].y,
                0.0};

            vertices[3] = glm::vec3{
                vertices[2].x,
                vertices[1].y,
                0.0};

            for (auto &vertex : vertices)
            {
                vertex.x = calculateScreenCoordX(vertex.x);
                vertex.y = calculateScreenCoordY(vertex.y);
            }

            for (auto &index : {0, 1, 2, 2, 1, 3})
            {
                vertices_buffer.push_back(vertices[index].x);
                vertices_buffer.push_back(vertices[index].y);
                vertices_buffer.push_back(vertices[index].z);
            }

            glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo_);
            glBufferData(GL_ARRAY_BUFFER, vertices_buffer.size() * sizeof(GLfloat),
                         vertices_buffer.data(), GL_DYNAMIC_DRAW);
            enableDepthTesting(false);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            enableDepthTesting(true);

            cursor_pos_x += font_face_->glyph->advance.x >> 6;

            vertices_buffer.clear();
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }

protected:
    FT_Face font_face_;
    FT_Library ft_library_;
    GLuint font_texture_handle_{0};
    GLuint handle_{0};
    GLuint texture_coords_vbo_{0};
    GLuint vertices_vbo_{0};
    unsigned int viewport_height_{800};
    unsigned int viewport_width_{600};

};
//*************************************************************************************************
int main()
{
    // Create main window
    int result = createWindow(800, 600, "GL Window", 4, false);
    if (result)
        return -1;

    // Load shaders
    // ----- MESH
    GLuint mesh_shader = 0;
    if (createShaderProgram(mesh_shader, "mesh_vs.glsl", "mesh_fs.glsl"))
        return -1;

    // ----- SKYBOX
    GLuint skybox_shader = 0;
    if (createShaderProgram(skybox_shader, "skybox_vs.glsl", "skybox_fs.glsl"))
        return -1;

    // ----- FONT
    GLuint font_shader = 0;
    if (createShaderProgram(font_shader, "font_vs.glsl", "font_fs.glsl"))
        return -1;

    // Find uniforms
    GLint texture_slot_mesh = findUniform(mesh_shader, "basic_texture");
    GLint view_uniform_mesh = findUniform(mesh_shader, "view_matrix");
    GLint projection_uniform_mesh = findUniform(mesh_shader, "projection_matrix");
    GLint model_uniform_mesh = findUniform(mesh_shader, "model_matrix");

    GLint view_uniform_sky = findUniform(skybox_shader, "view_matrix");
    GLint projection_uniform_sky = findUniform(skybox_shader, "projection_matrix");

    GLint texture_slot_font = findUniform(font_shader, "font_texture");
    GLint colour_font = findUniform(font_shader, "colour");

    // Configure camera
    aspect = float(window_width) / float(window_height);
    recalculateCamera();

    // Create skybox
    GLfloat skybox_vertices[] = {
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

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    GLuint skybox_vertices_vbo = 0;
    glGenBuffers(1, &skybox_vertices_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vertices_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);

    GLuint skybox_vao = 0;
    glGenVertexArrays(1, &skybox_vao);
    glBindVertexArray(skybox_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    GLuint skybox_texture = 0;
    loadTextureSkybox("hills_ft.tga", "hills_bk.tga", "hills_lf.tga", "hills_rt.tga",
                      "hills_up.tga", "hills_dn.tga", skybox_texture);

    // Load meshes
    MeshHandle city;
    loadSceneFromFile("city/city.obj", city);
    glm::mat4 mesh_model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1, 0.1, 0.1));

    // Create font renderer
    FreeTypeFontRenderer ft_font_renderer("/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 32);

    // Setup state machine
    enableFaceCulling(true);
    enableDepthTesting(true);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (renderingEnabled())
    {
        static double fps = 0;
        FPSCounter(fps);
        std::string title = "GL Window @ FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window_handle, title.c_str());

        updateTimer();

        clearColor(0.5, 0.5, 0.5);

        // Draw skybox
        activateShaderProgram(skybox_shader);
        glm::mat4 view_static = view_matrix;
        glm::vec3 pos(0.0, 0.0, 0.0);
        view_static = glm::lookAt(pos, pos + camera_direction, camera_up);
        setUniform(view_uniform_sky, view_static);
        setUniform(projection_uniform_sky, projection_matrix);

        enableDepthTesting(false);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        glBindVertexArray(skybox_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        enableDepthTesting(true);

        // Draw meshes
        activateShaderProgram(mesh_shader);
        setUniform(texture_slot_mesh, 0);
        setUniform(view_uniform_mesh, view_matrix);
        setUniform(projection_uniform_mesh, projection_matrix);
        setUniform(model_uniform_mesh, mesh_model_matrix);
        drawMesh(city);

        // Draw font
        activateShaderProgram(font_shader);
        setUniform(texture_slot_font, 0);
        setUniform(colour_font, glm::vec3(1.0, 1.0, 0.0));
        ft_font_renderer.renderText(L"Hello World!\nNew Line", 100, 50);
        setUniform(colour_font, glm::vec3(0.6, 0.2, 0.8));
        ft_font_renderer.renderText(L"Zażółć gęślą jaźń ...", 200, 200);
        setUniform(colour_font, glm::vec3(1.0, 0.1, 0.9));
        ft_font_renderer.renderText(L"\x410\x411\x412\x413\x414", 300, 400);
        setUniform(colour_font, glm::vec3(0.0, 1.0, 0.9));
        ft_font_renderer.renderText(L"\x3B2\x436\x2122\x263A", 50, 500);

        // Process window
        processWindowEvents();
        pollKeyboad();
        pollMouse();
    }

    terminate();

    std::system("pause");
    return 0;
}
//*************************************************************************************************
void setCursorPos(double x, double y)
{
    glfwSetCursorPos(window_handle, x, y);
}
//*************************************************************************************************
void windowSizeCallback(GLFWwindow *, int width, int height)
{
    window_width = width;
    window_height = height;

    aspect = float(window_width) / float(window_height);
    recalculateCamera();
}
//*************************************************************************************************
int createWindow(int width, int height, std::string name, int samples, bool fullscreen)
{
    if (name.empty())
        name = "Unnamed Window";

    if (width == 0 || height == 0)
    {
        std::cout << "Invalid window's \"" << name << "\" size." << std::endl;
        return -1;
    }

    if (!glfwInit())
    {
        std::cout << "GLFW initialization error." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, samples);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window_handle = glfwCreateWindow(width, height, name.c_str(),
                                     fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

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

    glfwSetWindowSizeCallback(window_handle, windowSizeCallback);

    return 0;
}
//*************************************************************************************************
void closeWindow(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, GL_TRUE);
}
//*************************************************************************************************
int createShaderProgram(GLuint &handle, std::string vertex_shader_file,
                        std::string fragment_shader_file)
{
    GLuint vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);

    if (loadShader(vertex_shader_handle, vertex_shader_file, ShaderType::VERTEX_SHADER))
        return -1;

    if (loadShader(fragment_shader_handle, fragment_shader_file, ShaderType::FRAGMENT_SHADER))
        return -1;

    handle = glCreateProgram();

    if (linkShaderProgram(handle, vertex_shader_handle, fragment_shader_handle))
        return -1;

    std::cout << "Shader program created successfully." << std::endl;

    return 0;
}
//*************************************************************************************************
int loadShader(GLuint &shader_handle, std::string file_name, ShaderType shader_type)
{
    std::string shader_data;
    if (loadShaderCode(file_name, shader_data))
    {
        std::cout << "Error opening shader file \"" + file_name + "\"." << std::endl;
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
        std::cout << "Shader \"" << file_name << "\" compile error." << std::endl;
        std::cout << "Shader compile log:" << std::endl;
        std::cout << getShaderCompileMsg(shader_handle) << std::endl;
        return -1;
    }

    std::cout << "Shader file \"" << file_name << "\" compile success." << std::endl;

    return 0;
}
//*************************************************************************************************
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
//*************************************************************************************************
int compileShader(GLuint shader_handle)
{
    glCompileShader(shader_handle);

    if (checkShaderCompileStatus(shader_handle))
        return -1;

    return 0;
}
//*************************************************************************************************
int checkShaderCompileStatus(GLuint shader_handle)
{
    int shader_status = -1;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &shader_status);
    if (shader_status != GL_TRUE)
        return -1;

    return 0;
}
//*************************************************************************************************
std::string getShaderCompileMsg(GLuint shader_handle)
{
    const unsigned int buffer_size = 2048;
    int length = 0;
    char log_text[buffer_size];
    glGetShaderInfoLog(shader_handle, buffer_size, &length, log_text);
    std::string compile_msg = std::string(log_text);

    return compile_msg;
}
//*************************************************************************************************
int checkShaderProgramLinkStatus(GLuint shader_program)
{
    int link_status = -1;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE)
        return -1;

    return 0;
}
//*************************************************************************************************
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
//*************************************************************************************************
int findUniform(GLuint shader_program, std::string uniform_name)
{
    GLint result = glGetUniformLocation(shader_program, uniform_name.c_str());

    if (result == -1)
        std::cout << "Uniform \"" << uniform_name << "\" not found." << std::endl;

    return result;
}
//*************************************************************************************************
void recalculateCamera()
{
    camera_up = glm::cross(camera_right, camera_direction);
    view_matrix = glm::lookAt(camera_position, camera_position + camera_direction, camera_up);
    projection_matrix = glm::perspective(FOV, aspect, P1, P2);
}
//*************************************************************************************************
double getTimeDelta()
{
    return (actual_time - previous_time);
}
//*************************************************************************************************
void updateTimer()
{
    previous_time = actual_time;
    actual_time = glfwGetTime();
}
//*************************************************************************************************
void setCameraAngles(float horizontal, float vertical)
{
    camera_direction = glm::vec3(cos(vertical) * sin(horizontal), sin(vertical), cos(vertical) *
                                 cos(horizontal));
    camera_right = glm::vec3(-cos(horizontal), 0, sin(horizontal));
    camera_up = glm::cross(camera_right, camera_direction);

    view_matrix = glm::lookAt(camera_position, camera_position + camera_direction, camera_up);
}
//*************************************************************************************************
void loadTextureSkybox(std::string front, std::string back, std::string left, std::string right,
                       std::string up, std::string down, GLuint &texture_handle)
{
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_handle);

    std::string textures[] = {right, left, down, up, back, front};

    for (int i = 0; i < 6; i++)
    {
        Texture texture;
        loadTexture(textures[i], texture);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, texture.width, texture.height,
                     0, GL_BGR, GL_UNSIGNED_BYTE, texture.bits);

        freeTextureData(texture);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
//*************************************************************************************************
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
        std::cout << "Texture \"" << file_name << "\" has unknown file format." << std::endl;
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
//*************************************************************************************************
void freeTextureData(Texture &texture)
{
    FreeImage_Unload(texture.image_ptr);
}
//*************************************************************************************************
int loadSceneFromFile(std::string file_name, std::vector<Mesh*>& mesh_handle)
{
    const aiScene* scene = aiImportFile(file_name.c_str(), aiProcessPreset_TargetRealtime_Fast);
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

        for (unsigned int f = 0; f != mesh->mNumFaces; f++)
        {
            const aiFace *face = &mesh->mFaces[f];

            for (unsigned int v = 0; v != 3; v++)
            {
                aiVector3D position{0, 0, 0};
                aiVector3D normal_vector{0, 0, 0};
                aiVector3D texture_coords{0, 0, 0};

                if (mesh->HasPositions())
                    position = mesh->mVertices[face->mIndices[v]];

                if (mesh->HasNormals())
                    normal_vector = mesh->mNormals[face->mIndices[v]];

                if (mesh->HasTextureCoords(0))
                    texture_coords = mesh->mTextureCoords[0][face->mIndices[v]];

                position_container.push_back(position.x);
                position_container.push_back(position.y);
                position_container.push_back(position.z);

                normal_vector_container.push_back(normal_vector.x);
                normal_vector_container.push_back(normal_vector.y);
                normal_vector_container.push_back(normal_vector.z);

                texture_coord_container.push_back(texture_coords.x);
                texture_coord_container.push_back(texture_coords.y);
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

        glGenVertexArrays(1, &mesh_entity->handle);
        glBindVertexArray(mesh_entity->handle);

        glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, normal_vector_vbo);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, texture_coord_vbo);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        mesh_entity->vertices_count = position_container.size();

        if (scene->mNumMaterials != 0)
        {
            const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            aiString texture_path;

            GLuint texture_diffuse = 0;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == AI_SUCCESS)
            {
                unsigned int found_pos = file_name.find_last_of("/\\");
                std::string path = file_name.substr(0, found_pos);
                std::string name(texture_path.C_Str());
                if (name[0] == '/')
                    name.erase(0, 1);

                std::string file_path = path + "/" + name;

                Texture tex;
                if (loadTexture(file_path, tex))
                    std::cout << "Texture \"" << file_path << "\" not found." << std::endl;
                else
                {
                    loadTexture2D(texture_diffuse, tex);
                    std::cout << "Texture \"" << file_path << "\" loaded." << std::endl;
                }

                freeTextureData(tex);
            }

            mesh_entity->diffuse_texture = texture_diffuse;
        }

        complete_mesh.push_back(mesh_entity);
    }

    mesh_handle = complete_mesh;

    return 0;
}
//*************************************************************************************************
int loadTexture2D(GLuint& texture_handle, Texture texture)
{
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    unsigned int colours = FreeImage_GetBPP(texture.image_ptr);
    if (colours == 24)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_BGR,
                     GL_UNSIGNED_BYTE, texture.bits);
    else if (colours == 32)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_BGRA,
                     GL_UNSIGNED_BYTE, texture.bits);

    glGenerateMipmap(GL_TEXTURE_2D);

    GLfloat anisotropy_factor = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy_factor);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy_factor);

    glBindTexture(GL_TEXTURE_2D, 0);

    return 0;
}
//*************************************************************************************************
void enableDepthTesting(bool state)
{
    glDepthFunc(GL_LESS);

    if (state)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}
//*************************************************************************************************
void enableFaceCulling(bool state)
{
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    if (state)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}
//*************************************************************************************************
bool renderingEnabled()
{
    if (glfwWindowShouldClose(window_handle))
        return false;

    return true;
}
//*************************************************************************************************
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
//*************************************************************************************************
void clearColor(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window_width, window_height);
}
//*************************************************************************************************
void activateShaderProgram(GLuint shader_program)
{
    glUseProgram(shader_program);
}
//*************************************************************************************************
void setUniform(GLint uniform_handle, GLint value)
{
    glUniform1iv(uniform_handle, 1, &value);
}
//*************************************************************************************************
void setUniform(GLint uniform_handle, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(uniform_handle, 1, GL_FALSE, glm::value_ptr(matrix));
}
//*************************************************************************************************
void setUniform(GLint uniform_handle, const glm::vec3 &vector)
{
    glUniform3fv(uniform_handle, 1, glm::value_ptr(vector));
}
//*************************************************************************************************
void drawMesh(const MeshHandle& mesh)
{
    for (const auto &it : mesh)
    {
        glBindVertexArray(it->handle); // TODO: Fix it !
        // Diffuse texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, it->diffuse_texture);

        glDrawArrays(GL_TRIANGLES, 0, it->vertices_count);
        glBindVertexArray(0);
    }
}
//*************************************************************************************************
void processWindowEvents()
{
    glfwSwapBuffers(window_handle);
    glfwPollEvents();
}
//*************************************************************************************************
void terminate()
{
    glfwDestroyWindow(window_handle);
    glfwTerminate();
}
//*************************************************************************************************
void pollKeyboad()
{
    if (glfwGetKey(window_handle, GLFW_KEY_ESCAPE))
        closeWindow(window_handle);

    bool camera_moved = false;
    float move_speed = 0.5f;

    if (glfwGetKey(window_handle, GLFW_KEY_A))
    {
        camera_position -= camera_right * move_speed;
        camera_moved = true;
    }

    if (glfwGetKey(window_handle, GLFW_KEY_D))
    {
        camera_position += camera_right * move_speed;
        camera_moved = true;
    }

    if (glfwGetKey(window_handle, GLFW_KEY_W))
    {
        camera_position += camera_direction * move_speed;
        camera_moved = true;
    }

    if (glfwGetKey(window_handle, GLFW_KEY_S))
    {
        camera_position -= camera_direction * move_speed;
        camera_moved = true;
    }

    if (camera_moved)
        recalculateCamera();
}
//*************************************************************************************************
void pollMouse()
{
    static double cursor_x = 0;
    static double cursor_y = 0;

    if (glfwGetMouseButton(window_handle, GLFW_MOUSE_BUTTON_LEFT))
    {
        double cur_x;
        double cur_y;
        glfwGetCursorPos(window_handle, &cur_x, &cur_y);

        double x_diff = cursor_x - cur_x;
        double y_diff = cursor_y - cur_y;

        float mouse_speed = 0.01f;

        camera_horizontal_angle += (x_diff * mouse_speed);
        camera_vertical_angle += (y_diff * mouse_speed);

        if (camera_vertical_angle >= glm::half_pi<float>())
            camera_vertical_angle = glm::half_pi<float>();
        else if (camera_vertical_angle <= -glm::half_pi<float>())
            camera_vertical_angle = -glm::half_pi<float>();

        setCameraAngles(camera_horizontal_angle, camera_vertical_angle);
        recalculateCamera();

        cursor_x = cur_x;
        cursor_y = cur_y;
    }
    else
    {
        glfwGetCursorPos(window_handle, &cursor_x, &cursor_y);
    }
}
//*************************************************************************************************