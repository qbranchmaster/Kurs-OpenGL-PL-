#version 330
in vec3 texture_coordinates;

uniform samplerCube cube_texture;

out vec4 frag_colour;

void main()
{
    frag_colour = texture(cube_texture, texture_coordinates);
}
