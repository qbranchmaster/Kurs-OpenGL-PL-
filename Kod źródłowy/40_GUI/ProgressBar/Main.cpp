//******************************************************************************
// Kurs OpenGL - Krok po kroku
// http://kurs-opengl.pl
// Autor: Sebastian Tabaka
//******************************************************************************
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <FreeImage.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
//******************************************************************************
GLFWwindow *window_handle = nullptr;
int window_height = 0;
int window_width = 0;
std::string window_caption = "GL Window";

enum class ShaderType
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
};

double actual_time = 0;
double previous_time = 0;

float P1 = 0.1f;
float P2 = 100.0f;
float FOV = 0.57f;
float aspect = 16.0f / 9.0f;
float camera_horizontal_angle = 0.0f;
float camera_vertical_angle = 0.0f;

glm::vec3 camera_position(0.0f, 0.0f, -5.0f);
glm::vec3 camera_direction(0.0f, 0.0f, 1.0f);
glm::vec3 camera_right(-1.0f, 0.0f, 0.0f);
glm::vec3 camera_up;
glm::mat4 view_matrix;
glm::mat4 projection_matrix;

struct Texture
{
    BYTE *bits;
    FIBITMAP *image_ptr;
    int width;
    int height;
};

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
int loadShader(GLuint &shader_handle, std::string file_name, ShaderType shader_type);
int loadShaderCode(std::string file_name, std::string &shader_code);
int loadTexture(std::string file_name, Texture &texture);
int loadTexture2D(GLuint& texture_handle, Texture texture);
std::string getShaderCompileMsg(GLuint shader_handle);
void activateShaderProgram(GLuint shader_program);
void clearColor(float r, float g, float b);
void closeWindow(GLFWwindow *window);
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
void setUniform(GLint uniform_handle, GLint value);
void setUniform(GLint uniform_handle, const glm::mat4 &matrix);
void setUniform(GLint uniform_handle, const glm::vec3 &vector);
void terminate();
void updateTimer();

class GUIElement
{
public:
    GUIElement(int x, int y, int width, int height)
    {
        if (x < 0 || y < 0)
            throw std::string("Specified invalid GUI element's position.");

        if (height <= 0 || width <= 0)
            throw std::string("Specified invalid GUI element's size.");

        x_ = x;
        y_ = y;

        height_ = height;
        width_ = width;

        glGenVertexArrays(1, &handle_);

        glGenBuffers(1, &colours_vbo_);
        glGenBuffers(1, &vertices_vbo_);
        glGenBuffers(1, &indices_vbo_);

        glBindVertexArray(handle_);
        glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo_);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, colours_vbo_);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        border_vertices_.resize(4);
        border_vertices_[0] = glm::vec3(x_, y_, 0.0f);
        border_vertices_[1] = glm::vec3(border_vertices_[0].x, border_vertices_[0].y + height_,
                                        0.0f);
        border_vertices_[2] = glm::vec3(border_vertices_[0].x + width_, border_vertices_[0].y,
                                        0.0f);
        border_vertices_[3] = glm::vec3(border_vertices_[2].x, border_vertices_[1].y, 0.0f);

        for (auto &vertex : border_vertices_)
        {
            vertex.x = calculateScreenCoordX(vertex.x);
            vertex.y = calculateScreenCoordY(vertex.y);
        }
    }

    virtual ~GUIElement()
    {
        glDeleteBuffers(1, &colours_vbo_);
        glDeleteBuffers(1, &vertices_vbo_);
        glDeleteBuffers(1, &indices_vbo_);

        glDeleteVertexArrays(1, &handle_);
    }

    void setBackgroundColours(const glm::vec3 &colour1, const glm::vec3 &colour2)
    {
        background_colour_1_ = colour1;
        background_colour_2_ = colour2;
    }

    void setBorderColour(const glm::vec3 &colour)
    {
        border_colour_ = colour;
    }

    static void setViewportSize(int width, int height)
    {
        if (width > 0 && height > 0)
        {
            viewport_height_ = height;
            viewport_width_ = width;
        }
    }

    virtual void render() = 0;

protected:
    float calculateScreenCoordX(float x)
    {
        return (x / static_cast<float>(viewport_width_) * 2 - 1);
    }

    float calculateScreenCoordY(float y)
    {
        return (1 - y / static_cast<float>(viewport_height_) * 2);
    }

protected:
    static int viewport_height_;
    static int viewport_width_;

    glm::vec3 background_colour_1_{0.5f, 0.5f, 0.5f};
    glm::vec3 background_colour_2_{0.8f, 0.8f, 0.8f};
    glm::vec3 border_colour_{0.0f, 0.0f, 0.0f};
    GLuint colours_vbo_{0};
    GLuint gui_shader_{0};
    GLuint handle_{0};
    GLuint indices_vbo_{0};
    GLuint vertices_vbo_{0};
    int height_{0};
    int width_{0};
    int x_{0};
    int y_{0};
    std::vector<glm::vec3> border_vertices_;
    const std::vector<GLuint> border_indices_{0, 1, 3, 2};
    const std::vector<GLuint> fill_indices_{0, 1, 2, 2, 1, 3};

};

class GUIProgressBar : public GUIElement
{
public:
    GUIProgressBar(int x, int y, int width, int height) : GUIElement{x, y, width, height}
    {
        createShaderProgram(gui_shader_, "gui_progressbar_vs.glsl", "gui_progressbar_fs.glsl");

        alpha_uniform_ = glGetUniformLocation(gui_shader_, "alpha_factor");
        border_colour_uniform_ = glGetUniformLocation(gui_shader_, "border_colour");
        rendering_border_uniform_ = glGetUniformLocation(gui_shader_, "rendering_border");
    }

    void render()
    {
        int progress_end_x = x_ + value_ / 100.0 * width_;

        std::vector<glm::vec3> progress_bar_vertices(4);
        progress_bar_vertices[0] = glm::vec3(x_, y_, 0.0f);
        progress_bar_vertices[1] = glm::vec3(progress_bar_vertices[0].x,
                                             progress_bar_vertices[0].y + height_,
                                             0.0f);
        progress_bar_vertices[2] = glm::vec3(progress_end_x, progress_bar_vertices[0].y, 0.0f);
        progress_bar_vertices[3] = glm::vec3(progress_bar_vertices[2].x, progress_bar_vertices[1].y,
                                             0.0f);

        for (auto &vertex : progress_bar_vertices)
        {
            vertex.x = calculateScreenCoordX(vertex.x);
            vertex.y = calculateScreenCoordY(vertex.y);
        }

        std::vector<GLfloat> vertices_buffer;
        
        for (auto &vertex : border_vertices_)
        {
            vertices_buffer.push_back(vertex.x);
            vertices_buffer.push_back(vertex.y);
            vertices_buffer.push_back(vertex.z);
        }
        
        for (auto &vertex : progress_bar_vertices)
        {
            vertices_buffer.push_back(vertex.x);
            vertices_buffer.push_back(vertex.y);
            vertices_buffer.push_back(vertex.z);
        }        

        std::vector<glm::vec3> colours(8);
        colours[0] = background_colour_1_;
        colours[1] = background_colour_2_;
        colours[2] = background_colour_1_;
        colours[3] = background_colour_2_;
        colours[4] = bar_colour_1_;
        colours[5] = bar_colour_1_;
        colours[6] = bar_colour_2_;
        colours[7] = bar_colour_2_;

        std::vector<GLfloat> colours_buffer;
        for (auto &colour : colours)
        {
            colours_buffer.push_back(colour.r);
            colours_buffer.push_back(colour.g);
            colours_buffer.push_back(colour.b);
        }

        

        glBindVertexArray(handle_);

        glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices_buffer.size() * sizeof(GLfloat),
                     vertices_buffer.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, colours_vbo_);
        glBufferData(GL_ARRAY_BUFFER, colours_buffer.size() * sizeof(GLfloat),
                     colours_buffer.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo_);

        glUseProgram(gui_shader_);
        glUniform1i(alpha_uniform_, alpha_);

        glDisable(GL_DEPTH_TEST);
        // Narysuj tlo elementu gui
        glUniform1i(rendering_border_uniform_, 0); // Rysujemy elementy dwukolorowe
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, fill_indices_.size() * sizeof(GLuint),
                     fill_indices_.data(), GL_DYNAMIC_DRAW);
        glDrawElements(GL_TRIANGLES, fill_indices_.size(), GL_UNSIGNED_INT, 0);
        // Narysuj pasek postepu
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bar_indices.size() * sizeof(GLuint),
                     bar_indices.data(), GL_DYNAMIC_DRAW);
        glDrawElements(GL_TRIANGLES, bar_indices.size(), GL_UNSIGNED_INT, 0);
        // Narysuj ramke dookola elementu GUI
        glUniform1i(rendering_border_uniform_, 1); // Rysujemy elementy jednokolorowe
        glUniform3fv(border_colour_uniform_, 1, glm::value_ptr(border_colour_));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, border_indices_.size() * sizeof(GLuint),
                     border_indices_.data(), GL_DYNAMIC_DRAW);
        glDrawElements(GL_LINE_LOOP, border_indices_.size(), GL_UNSIGNED_INT, 0);

        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    void setAlpha(int value)
    {
        alpha_ = value;
        
        if (alpha_ > 100)
            alpha_ = 100;
        if (alpha_ < 0)
            alpha_ = 0;
    }

    void setBarColours(const glm::vec3 &colour1, const glm::vec3 &colour2)
    {
        bar_colour_1_ = colour1;
        bar_colour_2_ = colour2;
    }

    void setValue(int value)
    {
        value_ = value;

        if (value_ > 100)
            value_ = 100;
        if (value_ < 0)
            value_ = 0;
    }

protected:
    GLint alpha_uniform_{0};
    GLint border_colour_uniform_{0};
    GLint rendering_border_uniform_{0};
    glm::vec3 bar_colour_1_{0.8f, 0.0f, 0.0f};
    glm::vec3 bar_colour_2_{0.6f, 0.0f, 0.0f};
    int alpha_{100};
    int value_{0};
    const std::vector<GLuint> bar_indices{4, 5, 6, 6, 5, 7};

};

int GUIElement::viewport_height_;
int GUIElement::viewport_width_;
//*************************************************************************************************
int main()
{
    // Utworzenie okna
    int result = createWindow(800, 600, window_caption, 4, false);
    if (result)
        return -1;

    // Zaladuj shadery
    // ----- SKYBOX
    GLuint skybox_shader = 0;
    if (createShaderProgram(skybox_shader, "skybox_vs.glsl", "skybox_fs.glsl"))
        return -1;

    // Znajdz lokalizacje zmiennych uniform    
    GLint view_uniform_sky = findUniform(skybox_shader, "view_matrix");
    GLint projection_uniform_sky = findUniform(skybox_shader, "projection_matrix");

    // Konfiguracja kamery
    aspect = float(window_width) / float(window_height);
    recalculateCamera();

    // Stworz skybox
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
    loadTextureSkybox("craterlake_ft.tga", "craterlake_bk.tga", "craterlake_lf.tga", 
                      "craterlake_rt.tga", "craterlake_up.tga", "craterlake_dn.tga", 
                      skybox_texture);

    // Stworz GUI
    GUIElement::setViewportSize(window_width, window_height);
    GUIProgressBar progress_bar(100, 300, 300, 50);
    progress_bar.setAlpha(65);
    progress_bar.setValue(0);
    progress_bar.setBackgroundColours(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.2f, 0.2f, 0.2f));
    progress_bar.setBorderColour(glm::vec3(1.0f, 1.0f, 0.0f));
    progress_bar.setBarColours(glm::vec3(1.0f, 0.80f, 0.06f), glm::vec3(1.0f, 0.90f, 0.55f));

    GUIProgressBar progress_bar_2(300, 500, 150, 25);
    progress_bar_2.setAlpha(85);
    progress_bar_2.setValue(80);
    progress_bar_2.setBackgroundColours(glm::vec3(0.68f, 0.79f, 0.88f), glm::vec3(0.32f, 0.55f, 0.75f));
    progress_bar_2.setBorderColour(glm::vec3(0.0f, 0.0f, 0.65f));
    progress_bar_2.setBarColours(glm::vec3(0.0f, 0.50f, 0.0f), glm::vec3(0.0f, 0.87f, 0.0f));

    // Wstepna konfiguracja maszyny stanu
    enableFaceCulling(true);
    enableDepthTesting(true);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (renderingEnabled())
    {
        static double fps = 0.0f;
        FPSCounter(fps);
        std::string title = window_caption + " @ FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window_handle, title.c_str());

        updateTimer();

        clearColor(0.5f, 0.5f, 0.5f);

        // Narysuj skybox
        activateShaderProgram(skybox_shader);
        glm::mat4 view_static = view_matrix;
        glm::vec3 pos(0.0f, 0.0f, 0.0f);
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

        // Narysuj GUI
        static int value = 0;
        static double time_0 = glfwGetTime();
        double time_1 = glfwGetTime();
        if (time_1 - time_0 >= 0.5)
        {
            time_0 = time_1;
            value += 5;

            if (value > 100)
                value = 0;

            progress_bar.setValue(value);
        }

        progress_bar.render();
        progress_bar_2.render();

        // Przechwyc zdarzenia
        processWindowEvents();
        pollKeyboad();
        pollMouse();
    }

    terminate();

#ifdef WIN32
    std::system("pause");
#endif
    return 0;
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
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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