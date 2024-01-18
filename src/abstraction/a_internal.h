#pragma once

#include <rebound.h>

/*=========================*/
// Shader
/*=========================*/

typedef struct gl_shader_t gl_shader_t;
struct gl_shader_t {
    u32_t handle;
};

extern gl_shader_t gl_shader_new(re_str_t vert_source, re_str_t frag_source);
extern gl_shader_t gl_shader_file(const char *vert_path, const char *frag_path);
extern void gl_shader_free(gl_shader_t *shader);

extern void gl_shader_use(const gl_shader_t *shader);

/*=========================*/
// Buffers
/*=========================*/

typedef enum {
    GL_BUFFER_USAGE_STATIC,
    GL_BUFFER_USAGE_DYNAMIC,
} gl_buffer_usage_t;

typedef struct gl_vertex_buffer_t gl_vertex_buffer_t;
struct gl_vertex_buffer_t {
    u32_t handle;
    u32_t stride;
};

extern gl_vertex_buffer_t gl_vertex_buffer_new(u32_t size, u32_t stride, const void *data, gl_buffer_usage_t usage);
extern void gl_vertex_buffer_free(gl_vertex_buffer_t *vb);

typedef struct gl_index_buffer_t gl_index_buffer_t;
struct gl_index_buffer_t {
    u32_t handle;
};

extern gl_index_buffer_t gl_index_buffer_new(u32_t size, const u32_t *data, gl_buffer_usage_t usage);
extern void gl_index_buffer_free(gl_index_buffer_t *ib);

typedef enum {
    GL_VERTEX_ATTRIBUTE_TYPE_FLOAT,
    GL_VERTEX_ATTRIBUTE_TYPE_VEC2,
    GL_VERTEX_ATTRIBUTE_TYPE_VEC3,
    GL_VERTEX_ATTRIBUTE_TYPE_VEC4,
} gl_vertex_attribute_type_t;

typedef struct gl_vertex_attribute_t gl_vertex_attribute_t;
struct gl_vertex_attribute_t {
    gl_vertex_attribute_type_t type;
    u64_t offset;
};

typedef struct gl_vertex_array_t gl_vertex_array_t;
struct gl_vertex_array_t {
    u32_t handle;

    b8_t has_vb;
    gl_vertex_buffer_t vb;
    b8_t has_ib;
    gl_index_buffer_t ib;
};

extern gl_vertex_array_t gl_vertex_array_new(void);
extern void gl_vertex_array_free(gl_vertex_array_t *va);
extern void gl_vertex_array_attach_vertex_buffer(gl_vertex_array_t *va, gl_vertex_buffer_t vb, const gl_vertex_attribute_t *attributes, u32_t attribute_count);
extern void gl_vertex_array_attach_index_buffer(gl_vertex_array_t *va, gl_index_buffer_t ib);
extern void gl_vertex_array_draw_array(gl_vertex_array_t va, u32_t count, u64_t offset);
extern void gl_vertex_array_draw_index(gl_vertex_array_t va, u32_t count, u64_t offset);
