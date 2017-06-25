#version 330
in vec2 texture_coordinates;
uniform sampler2D basic_texture;
out vec4 frag_colour;
 
void main()
{
   vec4 texel = texture(basic_texture, texture_coordinates);

   if (texel.r > 0.46 && texel.r < 0.54 &&
       texel.g > 0.49 && texel.g < 0.57 &&
       texel.b > 0.36 && texel.b < 0.44)
   {
      discard;
   }

   frag_colour = texel;
}