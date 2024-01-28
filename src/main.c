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

typedef struct mesh_t mesh_t;
struct mesh_t {
    u32_t vao;
    b8_t indexed;
    u32_t ebo;
    u32_t index_count;
    u64_t index_offset;
    u32_t index_type;
};

typedef struct model_t model_t;
struct model_t {
    u32_t *buffers;
    u32_t buffer_count;
    u32_t *vaos;
    u32_t vao_count;

    // There is one mesh per VAO.
    mesh_t *meshes;
};

static void set_vertex_attribute(gltf_model_t model, u32_t *buffers, i32_t accessor, u32_t index) {
    if (accessor < 0) {
        return;
    }

    gltf_accessor_t acc = model.accessors[accessor];
    gltf_buffer_view_t view = model.views[acc.view];
    glBindBuffer(view.target, buffers[acc.view]);

    u32_t count = 0;
    switch (acc.type) {
        case GLTF_ACCESSOR_TYPE_SCALAR: count = 1;  break;
        case GLTF_ACCESSOR_TYPE_VEC2:   count = 2;  break;
        case GLTF_ACCESSOR_TYPE_VEC3:   count = 3;  break;
        case GLTF_ACCESSOR_TYPE_VEC4:   count = 4;  break;
        case GLTF_ACCESSOR_TYPE_MAT2:   count = 4;  break;
        case GLTF_ACCESSOR_TYPE_MAT3:   count = 9;  break;
        case GLTF_ACCESSOR_TYPE_MAT4:   count = 16; break;
    }

    glVertexAttribPointer(
            index,
            count,
            acc.comp_type,
            acc.normalized,
            view.stride,
            (const void *) acc.offset);
    glEnableVertexAttribArray(index);

    glBindBuffer(view.target, 0);
}

model_t gltf_to_model(gltf_model_t model, re_arena_t *arena) {
    model_t m = {0};

    m.meshes = re_arena_push(arena, model.mesh_count * sizeof(mesh_t));
    m.vaos = re_arena_push(arena, model.mesh_count * sizeof(u32_t));
    m.buffers = re_arena_push(arena, model.view_count * sizeof(u32_t));

    m.vao_count = model.mesh_count;
    m.buffer_count = model.view_count;

    glGenVertexArrays(model.mesh_count, m.vaos);
    glGenBuffers(model.view_count, m.buffers);

    for (u32_t i = 0; i < model.view_count; i++) {
        if (model.views[i].target == 0) {
            continue;
        }

        gltf_buffer_view_t view = model.views[i];
        re_str_t buffer = model.buffers[view.buffer];

        glBindBuffer(view.target, m.buffers[i]);
        glBufferData(view.target, view.length, buffer.str + view.offset, GL_STATIC_DRAW);
        glBindBuffer(view.target, 0);
    }

    for (u32_t i = 0; i < model.mesh_count; i++) {
        m.meshes[i].vao = m.vaos[i];
        glBindVertexArray(m.vaos[i]);

        set_vertex_attribute(model, m.buffers, model.meshes[i].position_accessor, 0);
        set_vertex_attribute(model, m.buffers, model.meshes[i].normal_accessor, 1);
        set_vertex_attribute(model, m.buffers, model.meshes[i].uv_accessor, 2);

        if (model.meshes[i].indices_accessor != -1) {
            gltf_accessor_t acc = model.accessors[model.meshes[i].indices_accessor];

            m.meshes[i].indexed = true;
            m.meshes[i].ebo = m.buffers[acc.view];
            m.meshes[i].index_count = acc.count;
            m.meshes[i].index_offset = acc.offset;
            m.meshes[i].index_type = acc.comp_type;
        }
    }
    glBindVertexArray(0);

    return m;
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

    // gltf_model_t gltf_model = gltf_parse("resources/models/box/Box.gltf", arena);
    // gltf_model_t gltf_model = gltf_parse("resources/models/suzanne/Suzanne.gltf", arena);
    // gltf_model_t gltf_model = gltf_parse("resources/models/avocado/Avocado.gltf", arena);
    gltf_model_t gltf_model = gltf_parse("resources/models/damaged_helmet/DamagedHelmet.gltf", arena);
    model_t model = gltf_to_model(gltf_model, arena);

    gl_shader_t shader = gl_shader_file("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");

    HMM_Mat4 projection = HMM_Perspective_LH_NO(90.0f, 800.0f/600.0f, 0.1f, 10.0f);


    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        f32_t speed = 2.0f;
        f32_t angle = re_os_get_time() * speed;
        f32_t distance = 3.0f;

        HMM_Mat4 translation = HMM_Translate(HMM_V3(cosf(angle) * distance, 0.0f, sinf(angle) * distance));
        HMM_Mat4 rotation = HMM_Rotate_LH(-angle + RAD(90.0f), HMM_V3(0.0f, 1.0f, 0.0f));

        HMM_Mat4 view = HMM_MulM4(rotation, translation);

        gl_shader_use(&shader);
        u32_t loc = glGetUniformLocation(shader.handle, "projection");
        glUniformMatrix4fv(loc, 1, false, &projection.Elements[0][0]);

        loc = glGetUniformLocation(shader.handle, "view");
        glUniformMatrix4fv(loc, 1, false, &view.Elements[0][0]);

        {
            HMM_Mat4 translation = HMM_Translate(HMM_V3(0.0f, 0.0f, 0.0f));
            // HMM_Mat4 rotation = HMM_Rotate_LH(re_os_get_time() * speed, HMM_V3(0.0f, 1.0f, 0.0f));
            HMM_Mat4 rotation = HMM_QToM4(HMM_Q(0.7f, 0.0f, 0.0f, 0.7f));
            // HMM_Mat4 rotation = HMM_M4D(1.0f);
            HMM_Mat4 scale = HMM_Scale(HMM_MulV3F(HMM_V3(1.0f, 1.0f, 1.0f), 1.0f));

            HMM_Mat4 transform = HMM_MulM4(translation, rotation);
            transform = HMM_MulM4(transform, scale);

            loc = glGetUniformLocation(shader.handle, "transform");
            glUniformMatrix4fv(loc, 1, false, &transform.Elements[0][0]);

            glBindVertexArray(model.meshes[0].vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.meshes[0].ebo);

            glDrawElements(
                    GL_TRIANGLES,
                    model.meshes[0].index_count,
                    model.meshes[0].index_type,
                    (const void *) model.meshes[0].index_offset);
        }

        // {
        //     HMM_Mat4 translation = HMM_Translate(HMM_V3(-1.0f, 0.0f, 3.0f));
        //     HMM_Mat4 rotation = HMM_Rotate_LH(re_os_get_time(), HMM_V3(1.0f, 1.0f, 0.0f));
        //     HMM_Mat4 scale = HMM_Scale(HMM_MulV3F(HMM_V3(1.0f, 1.0f, 1.0f), 25.0f));
        //
        //     HMM_Mat4 transform = HMM_MulM4(translation, rotation);
        //     transform = HMM_MulM4(transform, scale);
        //
        //     loc = glGetUniformLocation(shader.handle, "transform");
        //     glUniformMatrix4fv(loc, 1, false, &transform.Elements[0][0]);
        //
        //     glBindVertexArray(avo.meshes[0].vao);
        //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, avo.meshes[0].ebo);
        //
        //     glDrawElements(
        //             GL_TRIANGLES,
        //             avo.meshes[0].index_count,
        //             avo.meshes[0].index_type,
        //             (const void *) avo.meshes[0].index_offset);
        // }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    re_terminate();
    return 0;
}
