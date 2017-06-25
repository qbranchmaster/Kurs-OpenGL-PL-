#version 330
in vec3 vertex_colour;

out vec4 frag_colour;

uniform bool rendering_border;
uniform int alpha_factor;
uniform vec3 border_colour;

void main()
{
   if (rendering_border)
      frag_colour = vec4(border_colour, alpha_factor / 100.0);
   else
      frag_colour = vec4(vertex_colour, alpha_factor / 100.0);
}