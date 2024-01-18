#include "a_internal.h"

#include <glad/gl.h>

gl_index_buffer_t gl_index_buffer_new(u32_t size, const u32_t *data, gl_buffer_usage_t usage) {
    gl_index_buffer_t ib = {0};

    glGenBuffers(1, &ib.handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.handle);
    u32_t gl_usage = usage == GL_BUFFER_USAGE_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, gl_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ib;
}

void gl_index_buffer_free(gl_index_buffer_t *ib) {
    glDeleteBuffers(1, &ib->handle);
    *ib = (gl_index_buffer_t) {0};
}
