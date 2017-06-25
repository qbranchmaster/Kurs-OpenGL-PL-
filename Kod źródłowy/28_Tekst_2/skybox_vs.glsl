#version 330
layout(location = 0) in vec3 position;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
 
out vec3 texture_coordinates;
 
void main() 
{
    texture_coordinates = vec3(position.x, -position.yz);
    gl_Position = projection_matrix * view_matrix * vec4(position, 1.0);
}
