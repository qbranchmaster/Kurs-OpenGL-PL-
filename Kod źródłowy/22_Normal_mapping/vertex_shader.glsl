#version 330
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

uniform mat4 view_matrix;
uniform mat4 perspective_matrix;
uniform mat4 model_matrix;

out vec2 texture_coordinates;
out vec3 view_direction_tangent;
out vec3 light_direction_tangent;

vec3 light_pos_world = vec3(-4.0, 5.0, -10.0);

void main() 
{
    gl_Position = perspective_matrix * view_matrix * model_matrix * vec4(vertex_position, 1.0);
    texture_coordinates = texture_coord;

    vec3 camera_pos_world = (inverse(view_matrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;  
    vec3 camera_pos_local = vec3(inverse(model_matrix) * vec4(camera_pos_world, 1.0));
      
    vec3 light_pos_local = (inverse(model_matrix) * vec4(light_pos_world, 1.0)).xyz;
    vec3 light_dir_local = normalize(vertex_position - light_pos_local);
    
    vec3 view_dir_local = normalize(camera_pos_local - vertex_position);
    
    mat3 tbn = mat3(tangent, bitangent, vertex_normal);
    
    view_direction_tangent = inverse(tbn) * view_dir_local;
    light_direction_tangent = inverse(tbn) * light_dir_local;      
}
