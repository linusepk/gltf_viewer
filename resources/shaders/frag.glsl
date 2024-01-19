#version 330 core
layout (location = 0) out vec4 frag_color;

in vec4 f_color;

void main() {
    frag_color = f_color;
    // frag_color = vec4(1.0, 0.5, 0.2, 1.0);
}
