#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vt;

uniform mat4 view_matrix;
uniform mat4 perspective_matrix;
 
out vec2 texture_coordinates;
 
void main() 
{
   texture_coordinates = vt;
   gl_Position = perspective_matrix * view_matrix * vec4(position, 1.0);
}
