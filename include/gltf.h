#pragma once

#include <rebound.h>

typedef enum {
    GLTF_BUFFER_TARGET_ARRAY         = 34962,
    GLTF_BUFFER_TARGET_ELEMENT_ARRAY = 34963,
} gltf_buffer_target;

typedef struct gltf_buffer_view_t gltf_buffer_view_t;
struct gltf_buffer_view_t {
    u32_t buffer;
    u32_t offset;
    u32_t length;
    u32_t stride;
    gltf_buffer_target target;
};

typedef enum {
    GLTF_COMP_TYPE_BYTE           = 5120,
    GLTF_COMP_TYPE_UNSIGNED_BYTE  = 5121,
    GLTF_COMP_TYPE_SHORT          = 5122,
    GLTF_COMP_TYPE_UNSIGNED_SHORT = 5123,
    GLTF_COMP_TYPE_UNSIGNED_INT   = 5125,
    GLTF_COMP_TYPE_FLOAT          = 5126,
} gltf_comp_type_t;

typedef enum {
    GLTF_ACCESSOR_TYPE_SCALAR,
    GLTF_ACCESSOR_TYPE_VEC2,
    GLTF_ACCESSOR_TYPE_VEC3,
    GLTF_ACCESSOR_TYPE_VEC4,
    GLTF_ACCESSOR_TYPE_MAT2,
    GLTF_ACCESSOR_TYPE_MAT3,
    GLTF_ACCESSOR_TYPE_MAT4,
} gltf_accessor_type_t;

typedef struct gltf_accessor_t gltf_accessor_t;
struct gltf_accessor_t {
    u32_t view;
    u64_t offset;
    gltf_comp_type_t comp_type;
    b8_t normalized;
    u32_t count;
    gltf_accessor_type_t type;
};

typedef struct gltf_mesh_t gltf_mesh_t;
struct gltf_mesh_t {
    i32_t position_accessor;
    i32_t normal_accessor;
    i32_t uv_accessor;
    i32_t indices_accessor;
};

typedef struct gltf_model_t gltf_model_t;
struct gltf_model_t {
    re_str_t *buffers;
    u32_t buffer_count;

    gltf_buffer_view_t *views;
    u32_t view_count;

    gltf_accessor_t *accessors;
    u32_t accessor_count;

    gltf_mesh_t *meshes;
    u32_t mesh_count;
};

extern gltf_model_t gltf_parse(const char *path, re_arena_t *arena);
