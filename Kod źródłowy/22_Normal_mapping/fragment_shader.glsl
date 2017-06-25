#version 330
in vec2 texture_coordinates;
in vec3 view_direction_tangent;
in vec3 light_direction_tangent;

uniform mat4 view_matrix;
uniform sampler2D basic_texture;
uniform sampler2D normal_texture;

out vec4 frag_colour;

vec3 ambient_color = vec3(0.3, 0.3, 0.3);
vec3 diffuse_color = vec3(0.8, 0.8, 0.8);
vec3 specular_color = vec3(1.0, 1.0, 1.0);

vec3 object_ambient_factor = vec3(1.0, 1.0, 1.0);
vec3 object_diffuse_factor = vec3(1.0, 1.0, 1.0);
vec3 object_specular_factor = vec3(1.0, 1.0, 1.0);
 
void main()
{
    vec3 normal_tangent = texture(normal_texture, texture_coordinates).rgb;
    normal_tangent = normalize(normal_tangent * 2.0 - 1.0);
    
    // Ambient
    vec3 ambient_intense = ambient_color * object_ambient_factor;

    // Diffuse
    vec3 direction_to_light_tangent = normalize(-light_direction_tangent);
    float dot_product = dot(direction_to_light_tangent, normal_tangent);
    dot_product = max(dot_product, 0.0);
    vec3 diffuse_intense = diffuse_color * object_diffuse_factor * dot_product;

    // Specular
    vec3 reflection_tangent = reflect(normalize(light_direction_tangent), normal_tangent);
    float dot_specular = dot(reflection_tangent, normalize(-view_direction_tangent));
    dot_specular = max(dot_specular, 0.0);
    float specular_factor = pow(dot_specular, 10.0);
    vec3 specular_intense = specular_color * object_specular_factor * specular_factor;

    vec4 texel = texture(basic_texture, texture_coordinates);   
    frag_colour = vec4(diffuse_intense + specular_intense + ambient_intense, 1.0) * texel;
}
