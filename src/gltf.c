#include "gltf.h"
#include "rebound.h"

#include "json.h"

static re_str_t *parse_buffers(const json_object_t *root, re_str_t dir, re_arena_t *arena) {
    json_object_t buffers = json_object(*root, re_str_lit("buffers"));

    re_str_t *buffs = re_arena_push(arena, buffers.value.array.count * sizeof(re_str_t *));

    re_arena_temp_t scratch = re_arena_scratch_get(&arena, 1);
    for (u32_t i = 0; i < buffers.value.array.count; i++) {
        json_object_t buffer = json_array(buffers, i);
        re_str_t uri = json_string(json_object(buffer, re_str_lit("uri")));

        re_str_t buff_path = re_str_concat(dir, uri, scratch.arena);
        char *path = re_arena_push_zero(arena, buff_path.len + 1);
        for (u32_t i = 0; i < buff_path.len; i++) {
            path[i] = buff_path.str[i];
        }

        buffs[i] = re_file_read(path, arena);
    }

    re_arena_scratch_release(&scratch);

    return buffs;
}

static gltf_buffer_view_t *parse_views(const json_object_t *root, re_arena_t *arena) {
    json_object_t json_views = json_object(*root, re_str_lit("bufferViews"));

    gltf_buffer_view_t *views = re_arena_push(arena, json_views.value.array.count);
    for (u32_t i = 0; i < json_views.value.array.count; i++) {
        json_object_t view = json_array(json_views, i);

        u32_t buffer = json_int(json_object(view, re_str_lit("buffer"))); 

        u32_t offset = 0;
        json_object_t json_offset = json_object(view, re_str_lit("byteOffset"));
        if (json_offset.type != JSON_TYPE_ERROR) {
            offset = json_int(json_offset);
        }

        u32_t length = json_int(json_object(view, re_str_lit("byteLength")));

        u32_t stride = 0;
        json_object_t json_stride = json_object(view, re_str_lit("byteStride"));
        if (json_offset.type != JSON_TYPE_ERROR) {
            stride = json_int(json_stride);
        }

        gltf_buffer_target target = GLTF_BUFFER_TARGET_NONE;
        json_object_t json_target = json_object(view, re_str_lit("target"));
        if (json_offset.type != JSON_TYPE_ERROR) {
            target = json_int(json_target);
        }

        views[i] = (gltf_buffer_view_t) {
            buffer,
            offset,
            length,
            stride,
            target,
        };
    }

    return views;
}

static gltf_accessor_t *parse_accessors(const json_object_t *root, re_arena_t *arena) {
    json_object_t json_accs = json_object(*root, re_str_lit("accessors"));

    gltf_accessor_t *accessors = re_arena_push(arena, json_accs.value.array.count);
    for (u32_t i = 0; i < json_accs.value.array.count; i++) {
        json_object_t acc = json_array(json_accs, i);

        u32_t view = 0;
        json_object_t json_view = json_object(acc, re_str_lit("bufferView"));
        if (json_view.type != JSON_TYPE_ERROR) {
            view = json_int(json_view);
        }

        u32_t offset = 0;
        json_object_t json_offset = json_object(acc, re_str_lit("byteOffset"));
        if (json_offset.type != JSON_TYPE_ERROR) {
            offset = json_int(json_offset);
        }

        gltf_comp_type_t comp_type = json_int(json_object(acc, re_str_lit("componentType")));

        b8_t normalized = false;
        json_object_t json_normalized = json_object(acc, re_str_lit("normalized"));
        if (json_normalized.type != JSON_TYPE_ERROR) {
            normalized = json_bool(json_normalized);
        }

        u32_t count = json_int(json_object(acc, re_str_lit("count")));

        re_str_t str_type = json_string(json_object(acc, re_str_lit("type")));
        gltf_accessor_type_t type = 0;
        if      (re_str_cmp(str_type, re_str_lit("SCALAR")) == 0) { type = GLTF_ACCESSOR_TYPE_SCALAR; }
        else if (re_str_cmp(str_type, re_str_lit("VEC2"))   == 0) { type = GLTF_ACCESSOR_TYPE_VEC2; }
        else if (re_str_cmp(str_type, re_str_lit("VEC3"))   == 0) { type = GLTF_ACCESSOR_TYPE_VEC3; }
        else if (re_str_cmp(str_type, re_str_lit("VEC4"))   == 0) { type = GLTF_ACCESSOR_TYPE_VEC4; }
        else if (re_str_cmp(str_type, re_str_lit("MAT2"))   == 0) { type = GLTF_ACCESSOR_TYPE_MAT2; }
        else if (re_str_cmp(str_type, re_str_lit("MAT3"))   == 0) { type = GLTF_ACCESSOR_TYPE_MAT3; }
        else if (re_str_cmp(str_type, re_str_lit("MAT4"))   == 0) { type = GLTF_ACCESSOR_TYPE_MAT4; }
        else {
            re_log_error("Unknown accessor type %.*s.", (i32_t) str_type.len, str_type.str);
        }

        accessors[i] = (gltf_accessor_t) {
            view,
            offset,
            comp_type,
            normalized,
            count,
            type,
        };
    }

    return accessors;
}

gltf_model_t gltf_parse(const char *path, re_arena_t *arena) {
    re_arena_temp_t scratch = re_arena_scratch_get(&arena, 1);

    re_str_t file = re_file_read(path, scratch.arena);

    re_str_t _path = re_str_cstr(path);
    re_str_t dir = re_str_null;
    for (u32_t i = _path.len - 1; i > 0; i--) {
        if (_path.str[i] == '/') {
            dir = re_str_prefix(_path, i + 1);
            break;
        }
    }

    if (dir.str == NULL) {
        dir = re_str_lit("");
    }

    json_object_t json = json_parse(file);

    re_str_t *buffers = parse_buffers(&json, dir, arena);
    gltf_buffer_view_t *views = parse_views(&json, arena);
    gltf_accessor_t *accessors = parse_accessors(&json, arena);

    json_free(&json);

    re_arena_scratch_release(&scratch);

    return (gltf_model_t) {};
}
