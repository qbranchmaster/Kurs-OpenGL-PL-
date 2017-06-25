#version 330
in vec2 texture_coordinates;

out vec4 frag_colour;

uniform sampler2D font_texture;
 
void main()
{
    vec4 texel = texture(font_texture, texture_coordinates);   
    frag_colour = texel;
}