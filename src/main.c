#include <rebound.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

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

    gl_shader_t shader = gl_shader_file("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        gl_shader_use(&shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    re_terminate();
    return 0;
}
