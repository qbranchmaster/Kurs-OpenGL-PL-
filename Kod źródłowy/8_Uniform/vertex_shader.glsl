#version 330

layout(location = 0) in vec3 position;

uniform float trans;

void main() 
{
   gl_Position = vec4(position.x + trans, position.y + trans, position.z, 1.0);
}
