#version 330 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;

out vec3 f_normal;
out vec3 f_frag_coord;

uniform mat4 projection;
uniform mat4 transform;

void main() {
    gl_Position = projection * transform * vec4(v_position, 1.0);

    f_normal = mat3(transpose(inverse(transform))) * v_normal;
    f_frag_coord = vec3(transform * vec4(v_position, 1.0));
}
