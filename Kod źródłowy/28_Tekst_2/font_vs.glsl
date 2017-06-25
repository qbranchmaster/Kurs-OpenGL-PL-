#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vt;

out vec2 texture_coordinates;
 
void main() 
{
    texture_coordinates = vt;
    gl_Position = vec4(position, 1.0);
}