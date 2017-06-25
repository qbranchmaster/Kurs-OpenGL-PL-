#version 330
in vec2 texture_coordinates;

out vec4 frag_colour;

uniform sampler2D font_texture;
uniform vec3 colour;
 
void main()
{
    float texel_R = texture(font_texture, texture_coordinates).r; 
    frag_colour = vec4(1.0, 1.0, 1.0, texel_R) * vec4(colour, 1.0);
}
