#version 330
in vec2 texture_coordinates;

uniform sampler2D first_texture;
uniform sampler2D second_texture;

out vec4 frag_colour;
 
void main()
{
   vec4 first_sample = texture(first_texture, texture_coordinates);
   vec4 second_sample = texture(second_texture, texture_coordinates);

   frag_colour = mix(first_sample, second_sample, texture_coordinates.s);
}
