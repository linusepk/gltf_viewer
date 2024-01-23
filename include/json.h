#pragma once

#include <rebound.h>

typedef enum {
    JSON_TYPE_ERROR,
    JSON_TYPE_STRING,
    JSON_TYPE_FLOATING,
    JSON_TYPE_INTEGER,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_BOOL,
    JSON_TYPE_NULL,
} json_type_t;

typedef enum {
    JSON_ERROR_MISSING_COLON,
    JSON_ERROR_INVALID_VALUE,
    JSON_ERROR_MISSING_COMMA,
    JSON_ERROR_TYPE_MISMATCH,
    JSON_ERROR_ARRAY_OUT_OF_BOUNDS,
    JSON_ERROR_PROPERTY_NOT_FOUND,
} json_error_t;

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
        struct {
            json_object_t *values;
            u32_t count;
        } array;
        b8_t bool;
        void *null;

        struct {
            json_error_t type;
            u32_t line;
            u32_t offset;
        } error;
    } value;
};

extern json_object_t json_parse(re_str_t data);

extern re_str_t json_string(json_object_t obj);
extern f32_t json_float(json_object_t obj);
extern i32_t json_int(json_object_t obj);
extern f32_t json_number(json_object_t obj);
extern json_object_t json_object(json_object_t obj, re_str_t key);
extern json_object_t json_array(json_object_t obj, u32_t index);
extern b8_t json_bool(json_object_t obj);

extern json_object_t json_path(json_object_t obj, re_str_t path);
