#pragma once

#include <rebound.h>

typedef enum {
    JSON_TYPE_STRING,
    JSON_TYPE_FLOATING,
    JSON_TYPE_INTEGER,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_BOOL,
    JSON_TYPE_NULL,
} json_type_t;

typedef struct json_object_t json_object_t;
struct json_object_t {
    json_type_t type;
    union {
        re_str_t string;
        struct {
            re_str_t *keys;
            json_object_t *values;
            u32_t count;
        } object;
        f32_t floating;
        i32_t integer;
        b8_t bool;
        void *null;
    } value;
};

extern json_object_t json_parse(re_str_t data);
