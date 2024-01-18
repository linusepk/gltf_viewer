#include "a_internal.h"

#include <glad/gl.h>

gl_vertex_buffer_t gl_vertex_buffer_new(u32_t size, u32_t stride, const void *data, gl_buffer_usage_t usage) {
    gl_vertex_buffer_t vb = {0};
    vb.stride = stride;

    glGenBuffers(1, &vb.handle);
    glBindBuffer(GL_ARRAY_BUFFER, vb.handle);
    u32_t gl_usage = usage == GL_BUFFER_USAGE_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER, size, data, gl_usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vb;
}

void gl_vertex_buffer_free(gl_vertex_buffer_t *vb) {
    glDeleteBuffers(1, &vb->handle);
    *vb = (gl_vertex_buffer_t) {0};
}
