#version 330 core
layout (location = 0) out vec4 frag_color;

in vec3 f_normal;
in vec3 f_frag_coord;

void main() {
    vec4 color = vec4(1.0, 0.5, 0.2, 1.0);

    float ambient_strenght = 0.1;

    vec3 normal = normalize(f_normal);
    vec3 light_dir = vec3(0.2, 1.0, 0.3);
    float diff = max(dot(normal, light_dir), 0.0);

    frag_color = color * (ambient_strenght + diff);
}
