#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertex_colour;

uniform mat4 trans_matrix;

out vec3 colour;

void main() 
{
   colour = vertex_colour;
   gl_Position = trans_matrix * vec4(position, 1.0);
}
