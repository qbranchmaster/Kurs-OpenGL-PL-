#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal_vector;
layout(location = 2) in vec2 vt;

uniform mat4 view_matrix;
uniform mat4 perspective_matrix;
 
out vec2 texture_coordinates;
out vec3 vertex_to_camera;
out vec3 normal_to_camera;
 
void main() 
{
   normal_to_camera = vec3(view_matrix * vec4(normal_vector, 0.0));
   vertex_to_camera = vec3(view_matrix * vec4(position, 1.0));

   texture_coordinates = vt;
   gl_Position = perspective_matrix * view_matrix * vec4(position, 1.0);
}
