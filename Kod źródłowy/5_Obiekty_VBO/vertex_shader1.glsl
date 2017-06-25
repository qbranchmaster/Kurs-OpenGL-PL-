#version 330

layout(location = 0) in vec3 vp;

void main() 
{
   gl_Position = vec4(vp.x - 0.25, vp.y - 0.25, vp.z, 1.0);
}
