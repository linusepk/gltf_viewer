#include "a_internal.h"

#include <glad/gl.h>

gl_vertex_array_t gl_vertex_array_new(void) {
    gl_vertex_array_t va = {0};
    glGenVertexArrays(1, &va.handle);
    return va;
}

void gl_vertex_array_free(gl_vertex_array_t *va) {
    glDeleteVertexArrays(1, &va->handle);
    *va = (gl_vertex_array_t) {0};
}

void gl_vertex_array_attach_vertex_buffer(gl_vertex_array_t *va, gl_vertex_buffer_t vb, const gl_vertex_attribute_t *attributes, u32_t attribute_count) {
    va->vb = vb;
    va->has_vb = true;

    glBindVertexArray(va->handle);
    glBindBuffer(GL_ARRAY_BUFFER, vb.handle);
    for (u32_t i = 0; i < attribute_count; i++) {
        u32_t count = 0;

        switch (attributes[i].type) {
            case GL_VERTEX_ATTRIBUTE_TYPE_FLOAT: count = 1; break;
            case GL_VERTEX_ATTRIBUTE_TYPE_VEC2:  count = 2; break;
            case GL_VERTEX_ATTRIBUTE_TYPE_VEC3:  count = 3; break;
            case GL_VERTEX_ATTRIBUTE_TYPE_VEC4:  count = 4; break;
        }

        glVertexAttribPointer(i, count, GL_FLOAT, false, vb.stride, (const void *) attributes[i].offset);
        glEnableVertexAttribArray(i);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void gl_vertex_array_attach_index_buffer(gl_vertex_array_t *va, gl_index_buffer_t ib) {
    va->ib = ib;
    va->has_ib = true;
}

void gl_vertex_array_draw_array(gl_vertex_array_t va, u32_t count, u64_t offset) {
    glBindVertexArray(va.handle);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (const void *) (offset * sizeof(u32_t)));
    glBindVertexArray(0);
}

void gl_vertex_array_draw_index(gl_vertex_array_t va, u32_t count, u64_t offset) {
    glBindVertexArray(va.handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va.ib.handle);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (const void *) (offset * sizeof(u32_t)));
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
