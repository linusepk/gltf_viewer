#version 330 core
layout (location = 0) in vec3 v_pos;

uniform mat4 projection;
uniform mat4 transform;

void main() {
    gl_Position = projection * transform * vec4(v_pos, 1.0) + vec4(0.0, 0.0, -0.0, 0.0);
}
