#include <rebound.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "HandmadeMath.h"

#include "abstraction/a_internal.h"

#include "json.h"
#include "gltf.h"

static void resize_callback(GLFWwindow *window, i32_t width, i32_t height) {
    (void) window;
    glViewport(0, 0, width, height);
}

i32_t main(void) {
    re_init();
    re_arena_t *arena = re_arena_create(GB(4));

    if (!glfwInit()) {
        re_log_error("Failed to init GLFW.");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);

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

    gltf_model_t model = gltf_parse("resources/models/box/Box.gltf", arena);

    gl_shader_t shader = gl_shader_file("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");

    HMM_Mat4 projection = HMM_Perspective_LH_NO(90.0f, 800.0f/600.0f, 0.1f, 10.0f);

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gl_shader_use(&shader);
        u32_t loc = glGetUniformLocation(shader.handle, "projection");
        glUniformMatrix4fv(loc, 1, false, &projection.Elements[0][0]);

        HMM_Mat4 translation = HMM_Translate(HMM_V3(0.0f, 0.0f, 2.0f));
        HMM_Mat4 rotation = HMM_Rotate_LH(re_os_get_time(), HMM_V3(0.0f, 1.0f, 0.0f));
        HMM_Mat4 scale = HMM_Scale(HMM_V3(1.0f, 1.0f, 1.0f));

        HMM_Mat4 transform = HMM_MulM4(translation, rotation);
        transform = HMM_MulM4(transform, scale);

        loc = glGetUniformLocation(shader.handle, "transform");
        glUniformMatrix4fv(loc, 1, false, &transform.Elements[0][0]);

        glBindVertexArray(model.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);
        glDrawElements(GL_TRIANGLES, 36, 5123, NULL);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    re_terminate();
    return 0;
}
