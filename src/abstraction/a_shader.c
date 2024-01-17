#include "a_internal.h"
#include "rebound.h"

#include <glad/gl.h>

gl_shader_t gl_shader_new(re_str_t vert_source, re_str_t frag_source) {
    gl_shader_t shader = {0};

    i32_t success;
    char info_log[512];

    u32_t vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, (const char *const *) &vert_source.str, (const i32_t *) &vert_source.len);
    glCompileShader(vert_shader);
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vert_shader, 512, NULL, info_log);
        re_log_error("Vertex shader error.\n%s", info_log);
        return shader;
    }

    u32_t frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, (const char *const *) &frag_source.str, (const i32_t *) &frag_source.len);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(frag_shader, 512, NULL, info_log);
        re_log_error("Fragment shader error.\n%s", info_log);
        return shader;
    }

    u32_t program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        re_log_error("Shader linking error.\n%s", info_log);
        return shader;
    }
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    shader.handle = program;
    return shader;
}

gl_shader_t gl_shader_file(const char *vert_path, const char *frag_path) {
    b8_t error = false;
    re_arena_temp_t scratch = re_arena_scratch_get(NULL, 0);

    re_str_t vert_source = re_file_read(vert_path, scratch.arena);
    if (vert_source.str == NULL) {
        re_log_error("Couldn't open %s.", vert_path);
        error = true;
    }

    re_str_t frag_source = re_file_read(frag_path, scratch.arena);
    if (frag_source.str == NULL) {
        re_log_error("Couldn't open %s.", frag_path);
        error = true;
    }

    if (error) {
        return (gl_shader_t) {0};
    }

    gl_shader_t shader = gl_shader_new(vert_source, frag_source);

    re_arena_scratch_release(&scratch);

    return shader;
}

void gl_shader_free(gl_shader_t *shader) {
    glDeleteProgram(shader->handle);
    shader->handle = 0;
}

void gl_shader_use(const gl_shader_t *shader) {
    if (shader != NULL) {
        glUseProgram(shader->handle);
        return;
    }

    glUseProgram(0);
}
