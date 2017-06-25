#version 330

in vec3 vertex_colour;
out vec4 frag_colour;
  
void main()
{
   frag_colour = vec4(vertex_colour, 1.0);
}
