#version 330
in vec2 texture_coordinates;
in vec3 vertex_to_camera;
in vec3 normal_to_camera;

uniform mat4 view_matrix;
uniform sampler2D basic_texture;

out vec4 frag_colour;

vec3 light_direction = vec3(0.0, -1.0, 0.0);
vec3 ambient_color = vec3(0.2, 0.2, 0.2);
vec3 diffuse_color = vec3(0.8, 0.8, 0.8);

vec3 object_ambient_factor = vec3(1.0, 1.0, 1.0);
vec3 object_diffuse_factor = vec3(0.6, 0.6, 0.6);
 
void main()
{
   // Ambient
   vec3 ambient_intense = ambient_color * object_ambient_factor;  

   // Diffuse
   vec3 light_direction_CAMERA = (view_matrix * normalize(vec4(light_direction, 0.0))).xyz;
   vec3 direction_to_light = -light_direction_CAMERA;
   float dot_product = dot(direction_to_light, normal_to_camera);
   dot_product = max(dot_product, 0.0);
   vec3 diffuse_intense = diffuse_color * object_diffuse_factor * dot_product;  

   vec4 texel = texture(basic_texture, texture_coordinates);   
   frag_colour = vec4(ambient_intense + diffuse_intense, 1.0) * texel;  
}