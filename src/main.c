#include <rebound.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "HandmadeMath.h"

#include "abstraction/a_internal.h"

static void resize_callback(GLFWwindow *window, i32_t width, i32_t height) {
    (void) window;
    glViewport(0, 0, width, height);
}

i32_t main(void) {
    re_init();

    if (!glfwInit()) {
        re_log_error("Failed to init GLFW.");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Title", NULL, NULL);
    if (!window) {
        re_log_error("Failed to create window.");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        re_log_error("Failed load gl functions using GLAD.");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    typedef struct vert_t vert_t;
    struct vert_t {
        HMM_Vec3 position;
        HMM_Vec4 color;
    };
    vert_t verts[] = {
        {HMM_V3(-0.5f, -0.5f, 0.0f), HMM_V4(1.0f, 0.5f, 0.2f, 1.0f)},
        {HMM_V3( 0.5f, -0.5f, 0.0f), HMM_V4(1.0f, 0.5f, 0.2f, 1.0f)},
        {HMM_V3(-0.5f,  0.5f, 0.0f), HMM_V4(1.0f, 0.5f, 0.2f, 1.0f)},
        {HMM_V3( 0.5f,  0.5f, 0.0f), HMM_V4(1.0f, 0.5f, 0.2f, 1.0f)},
    };

    u32_t indices[] = {
        0, 1, 2,
        2, 3, 1,
    };

    gl_shader_t shader = gl_shader_file("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");

    gl_vertex_buffer_t vb = gl_vertex_buffer_new(sizeof(verts), sizeof(vert_t), verts, GL_BUFFER_USAGE_STATIC);
    gl_index_buffer_t ib = gl_index_buffer_new(sizeof(indices), indices, GL_BUFFER_USAGE_STATIC);

    gl_vertex_array_t va = gl_vertex_array_new();
    gl_vertex_attribute_t layout[] = {
        {GL_VERTEX_ATTRIBUTE_TYPE_VEC3, re_offsetof(vert_t, position)},
        {GL_VERTEX_ATTRIBUTE_TYPE_VEC4, re_offsetof(vert_t, color)},
    };
    gl_vertex_array_attach_vertex_buffer(&va, vb, layout, re_arr_len(layout));
    gl_vertex_array_attach_index_buffer(&va, ib);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        gl_shader_use(&shader);
        gl_vertex_array_draw_index(va, 6, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    re_terminate();
    return 0;
}
