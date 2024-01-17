#pragma once

#include <rebound.h>

typedef struct gl_shader_t gl_shader_t;
struct gl_shader_t {
    u32_t handle;
};

extern gl_shader_t gl_shader_new(re_str_t vert_source, re_str_t frag_source);
extern gl_shader_t gl_shader_file(const char *vert_path, const char *frag_path);
extern void gl_shader_free(gl_shader_t *shader);

extern void gl_shader_use(const gl_shader_t *shader);
