#include <rebound.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "HandmadeMath.h"

#include "abstraction/a_internal.h"

#include "json.h"

static void resize_callback(GLFWwindow *window, i32_t width, i32_t height) {
    (void) window;
    glViewport(0, 0, width, height);
}

static void json_print(json_object_t root, b8_t first_object, u32_t indent);

static void print_array(json_object_t arr, u32_t indent) {
    char space_buffer[512] = {0};
    for (u32_t i = 0; i < indent; i++) {
        space_buffer[i] = ' ';
    }

    for (u32_t i = 0; i < arr.value.array.count; i++) {
        json_object_t item = arr.value.array.values[i];

        switch (item.type) {
            case JSON_TYPE_STRING:
                re_log_debug("%s\"%.*s\"",
                        space_buffer,
                        (i32_t) item.value.string.len,
                        item.value.string.str);
                break;

            case JSON_TYPE_FLOATING:
                re_log_debug("%s%f",
                        space_buffer,
                        item.value.floating);
                break;

            case JSON_TYPE_INTEGER:
                re_log_debug("%s%d",
                        space_buffer,
                        item.value.integer);
                break;

            case JSON_TYPE_OBJECT:
                json_print(item, true, indent);
                break;

            case JSON_TYPE_ARRAY:
                re_log_debug("%s[", space_buffer);
                print_array(item, indent + 4);
                re_log_debug("%s]", space_buffer);
                break;

            case JSON_TYPE_BOOL:
                re_log_debug("%s%s",
                        space_buffer,
                        item.value.bool ? "true" : "false");
                break;

            case JSON_TYPE_NULL:
                re_log_debug("%snull",
                        space_buffer);
                break;

            case JSON_TYPE_ERROR:
                re_log_debug("Error");
                break;
        }
    }
}

static void json_print(json_object_t root, b8_t first_object, u32_t indent) {
    char space_buffer[512] = {0};
    if (first_object) {
        for (u32_t i = 0; i < indent; i++) {
            space_buffer[i] = ' ';
        }
        re_log_debug("%s{", space_buffer);
    }

    indent += 4;
    for (u32_t i = 0; i < indent; i++) {
        space_buffer[i] = ' ';
    }

    for (u32_t i = 0; i < root.value.object.count; i++) {
        re_str_t key = root.value.object.keys[i];
        json_object_t child = root.value.object.values[i];

        switch (child.type) {
            case JSON_TYPE_STRING:
                re_log_debug("%s\"%.*s\": \"%.*s\"",
                        space_buffer,
                        (i32_t) key.len, key.str,
                        (i32_t) child.value.string.len, child.value.string.str);
                break;

            case JSON_TYPE_FLOATING:
                re_log_debug("%s\"%.*s\": %f",
                        space_buffer,
                        (i32_t) key.len, key.str,
                        child.value.floating);
                break;

            case JSON_TYPE_INTEGER:
                re_log_debug("%s\"%.*s\": %d",
                        space_buffer,
                        (i32_t) key.len, key.str,
                        child.value.integer);
                break;

            case JSON_TYPE_OBJECT:
                re_log_debug("%s\"%.*s\": {",
                        space_buffer,
                        (i32_t) key.len, key.str);
                json_print(root.value.object.values[i], false, indent);
                break;

            case JSON_TYPE_ARRAY:
                re_log_debug("%s\"%.*s\": [",
                        space_buffer,
                        (i32_t) key.len, key.str);
                print_array(child, indent + 4);
                re_log_debug("%s]", space_buffer);
                break;

            case JSON_TYPE_BOOL:
                re_log_debug("%s\"%.*s\": %s",
                        space_buffer,
                        (i32_t) key.len, key.str,
                        child.value.bool ? "true" : "false");
                break;

            case JSON_TYPE_NULL:
                re_log_debug("%s\"%.*s\": null",
                        space_buffer,
                        (i32_t) key.len, key.str);
                break;

            case JSON_TYPE_ERROR:
                re_log_debug("Error");
                break;
        }
    }

    space_buffer[indent - 4] = '\0';
    re_log_debug("%s}", space_buffer);
}

i32_t main(void) {
    re_init();
    re_arena_t *arena = re_arena_create(GB(4));

    re_str_t json = re_file_read("resources/models/box/Box.gltf", arena);
    // re_str_t json = re_file_read("test.json", arena);
    json_object_t obj = json_parse(json);
    // json_print(obj, true, 0);

    json_object_t meshes = json_object(obj, re_str_lit("meshes"));
    json_object_t mesh = json_array(meshes, 0);
    json_object_t primitives = json_array(json_object(mesh, re_str_lit("primitives")), 0);
    json_object_t attributes = json_object(primitives, re_str_lit("attributes"));
    json_object_t normal = json_object(attributes, re_str_lit("POSITION"));
    i32_t norm = json_int(normal);
    re_log_debug("%d", norm);

    json_object_t value = json_path(obj, re_str_lit("meshes[0]/primitives[0]/attributes/NORMAL"));
    i32_t val = json_int(value);
    re_log_debug("%d", val);

    return 0;

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
