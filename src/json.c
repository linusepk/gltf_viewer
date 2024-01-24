#include "json.h"
#include <stdlib.h>

static b8_t is_digit(char c) {
    return c >= '0' && c <= '9';
}

typedef struct parser_t parser_t;
struct parser_t {
    u32_t i;
    re_str_t buffer;

    u32_t x;
    u32_t y;
};

static inline char peek(parser_t parser, u32_t offset) { return parser.buffer.str[parser.i + offset]; }
static inline void skip(parser_t *parser, u32_t count) { parser->x += count; parser->i += count; }
static inline char next(parser_t *parser) { skip(parser, 1); return peek(*parser, 0); }

static inline json_object_t json_parse_error(parser_t parser, json_error_t error) {
    return (json_object_t) {
        .type = JSON_TYPE_ERROR,
        .value.error = {
            .type = error,
            .offset = parser.x,
            .line = parser.y
        },
    };
}

static inline json_object_t json_error(json_error_t error) {
    return (json_object_t) {
        .type = JSON_TYPE_ERROR,
        .value.error = {
            .type = error,
            .offset = 0,
            .line = 0,
        },
    };
}


static void skip_whitespace(parser_t *parser) {
    while ((peek(*parser, 0) == ' ' ||
            peek(*parser, 0) == '\n' ||
            peek(*parser, 0) == '\r' ||
            peek(*parser, 0) == '\t') &&
            peek(*parser, 0) != '\0' &&
            parser->i < parser->buffer.len) {
        if (peek(*parser, 0) == '\n') {
            parser->x = 1;
            parser->y++;
        }
        skip(parser, 1);
    }
}

static json_object_t parse_object(parser_t *parser);

static json_object_t parse_string(parser_t *parser) {
    // Skip the first quote
    next(parser);

    u32_t start = parser->i;
    while (peek(*parser, 1) != '"') {next(parser);}
    u32_t end = parser->i;

    // Skip the last letter and the ending quote.
    skip(parser, 2);

    re_str_t sub = re_str_sub(parser->buffer, start, end);
    u8_t *str = re_malloc(sub.len);
    for (u32_t i = 0; i < sub.len; i++) {
        str[i] = sub.str[i];
    }
    return (json_object_t) {JSON_TYPE_STRING, {re_str(str, sub.len)}};
}

static json_object_t parse_bool(parser_t *parser) {
    json_object_t obj = {
        .type = JSON_TYPE_BOOL
    };

    switch (peek(*parser, 0)) {
        case 't':
            skip(parser, 4);
            obj.value.bool = true;
            break;
        case 'f':
            skip(parser, 5);
            obj.value.bool = false;
            break;
        default:
            obj.value.bool = -1;
            break;
    }

    return obj;
}

static json_object_t parse_null(parser_t *parser) {
    skip(parser, 4);
    return (json_object_t) {JSON_TYPE_NULL, .value.null = NULL};
}

static json_object_t parse_array(parser_t *parser);

static json_object_t parse_number(parser_t *parser) {
    skip_whitespace(parser);

    json_object_t obj = {
        .type = JSON_TYPE_INTEGER,
    };
    u32_t start = parser->i;
    while (is_digit(peek(*parser, 0)) || peek(*parser, 0) == '-' || peek(*parser, 0) == '.') {
        if (peek(*parser, 0) == '.') {
            obj.type = JSON_TYPE_FLOATING;
        }
        skip(parser, 1);
    }
    u32_t end = parser->i - 1;

    re_str_t num_str = re_str_sub(parser->buffer, start, end);

    switch (obj.type) {
        case JSON_TYPE_INTEGER:
            obj.value.integer = atoi((const char *) num_str.str);
            break;
        case JSON_TYPE_FLOATING:
            obj.value.floating = atof((const char *) num_str.str);
            break;
        default:
            break;
    }

    return obj;
}

static json_object_t parse_value(parser_t *parser) {
    skip_whitespace(parser);

    switch (peek(*parser, 0)) {
        case '"':
            return parse_string(parser);
        case '{':
            return parse_object(parser);
        case '[':
            return parse_array(parser);
        case 't':
        case 'f':
            return parse_bool(parser);
        case 'n':
            return parse_null(parser);
        default:
            break;
    }

    if (peek(*parser, 0) == '-' || is_digit(peek(*parser, 0))) {
        return parse_number(parser);
    }

    return json_parse_error(*parser, JSON_ERROR_INVALID_VALUE);
}

static json_object_t parse_array(parser_t *parser) {
    // Skip the [
    skip(parser, 1);

    json_object_t obj = {
        .type = JSON_TYPE_ARRAY,
        .value.array.count = 1,
    };
    obj.value.array.values = re_malloc(obj.value.array.count * sizeof(json_object_t));

    u32_t obj_i = 0;
    while (peek(*parser, 0) != ']') {
        skip_whitespace(parser);
        obj.value.array.values[obj_i] = parse_value(parser);

        skip_whitespace(parser);
        if (peek(*parser, 0) == ',') {
            skip(parser, 1);
            skip_whitespace(parser);
            if (peek(*parser, 0) == ']') {
                skip(parser, 1);
                return obj;
            }
            obj_i++;
            obj.value.array.count = obj_i + 1;
            obj.value.array.values = re_realloc(obj.value.array.values, obj.value.array.count * sizeof(json_object_t));
        }
    }
    skip(parser, 1);

    return obj;
}


static json_object_t parse_object(parser_t *parser) {
    // Skip the {
    skip(parser, 1);

    json_object_t obj = {
        .type = JSON_TYPE_OBJECT,
        .value.object.count = 1,
    };
    obj.value.object.keys = re_malloc(obj.value.object.count * sizeof(re_str_t));
    obj.value.object.values = re_malloc(obj.value.object.count * sizeof(json_object_t));

    u32_t obj_i = 0;
    while (peek(*parser, 0) != '}') {
        skip_whitespace(parser);
        if (peek(*parser, 0) != '"') {
            re_log_debug("%c", peek(*parser, 0));
            re_log_debug("%u:%u: Invalid key.", parser->y, parser->x); 
            return (json_object_t) {0};
        }
        obj.value.object.keys[obj_i] = parse_string(parser).value.string;

        skip_whitespace(parser);
        if (peek(*parser, 0) != ':') {
            return json_parse_error(*parser, JSON_ERROR_MISSING_COLON);
        } else {
            skip(parser, 1);
        }

        obj.value.object.values[obj_i] = parse_value(parser);

        skip_whitespace(parser);
        if (peek(*parser, 0) == ',') {
            skip(parser, 1);
            skip_whitespace(parser);
            if (peek(*parser, 0) == '}') {
                skip(parser, 1);
                return obj;
            }
            obj_i++;
            obj.value.object.count = obj_i + 1;
            obj.value.object.keys = re_realloc(obj.value.object.keys, obj.value.object.count * sizeof(re_str_t));
            obj.value.object.values = re_realloc(obj.value.object.values, obj.value.object.count * sizeof(json_object_t));
        } else {
            parser_t copy = *parser;
            skip_whitespace(&copy);
            if (peek(copy, 0) == '}') {
                skip(&copy, 1);
                *parser = copy;
                return obj;
            }

            return json_parse_error(*parser, JSON_ERROR_MISSING_COMMA);
        }
    }
    skip(parser, 1);

    return obj;
}

json_object_t json_parse(re_str_t data) {
    parser_t parser = {
        .x = 1,
        .y = 1,
    };
    parser.buffer = data;

    skip_whitespace(&parser);

    switch (peek(parser, 0)) {
        case '{':
            return parse_object(&parser);
        case '[':
            return parse_array(&parser);
        default:
            return (json_object_t) {0};
    }
}

static void json_free_array(json_object_t obj);

static void json_free_object(json_object_t obj) {
    if (obj.type != JSON_TYPE_OBJECT) {
        return;
    }

    for (u32_t i = 0; i < obj.value.object.count; i++) {
        switch (obj.value.object.values[i].type) {
            case JSON_TYPE_OBJECT:
                json_free_object(obj.value.object.values[i]);
                break;
            case JSON_TYPE_ARRAY:
                json_free_array(obj.value.object.values[i]);
                break;
            case JSON_TYPE_STRING:
                re_free(obj.value.object.values[i].value.string.str);
                break;
            default:
                break;
        }
    }

    re_free(obj.value.object.values);
    re_free(obj.value.object.keys);
}

static void json_free_array(json_object_t obj) {
    if (obj.type != JSON_TYPE_ARRAY) {
        return;
    }

    for (u32_t i = 0; i < obj.value.array.count; i++) {
        switch (obj.value.array.values[i].type) {
            case JSON_TYPE_OBJECT:
                json_free_object(obj.value.array.values[i]);
                break;
            case JSON_TYPE_ARRAY:
                json_free_array(obj.value.array.values[i]);
                break;
            case JSON_TYPE_STRING:
                re_free(obj.value.array.values[i].value.string.str);
                break;
            default:
                break;
        }
    }

    re_free(obj.value.array.values);
}

void json_free(json_object_t *root) {
    json_free_object(*root);
    json_free_array(*root);
    *root = (json_object_t) {JSON_TYPE_NULL, {0}};
}

re_str_t json_string(json_object_t obj) {
    if (obj.type != JSON_TYPE_STRING) {
        return re_str_null;
    }

    return obj.value.string;
}

f32_t json_float(json_object_t obj) {
    if (obj.type != JSON_TYPE_FLOATING) {
        return 0.0f;
    }

    return obj.value.floating;
}

i32_t json_int(json_object_t obj) {
    if (obj.type != JSON_TYPE_INTEGER) {
        return 0;
    }

    return obj.value.integer;
}

f32_t json_number(json_object_t obj) {
    if (obj.type != JSON_TYPE_INTEGER && obj.type != JSON_TYPE_FLOATING) {
        return 0.0f;
    }

    return obj.value.floating;
}

json_object_t json_object(json_object_t obj, re_str_t key) {
    if (obj.type != JSON_TYPE_OBJECT) {
        return json_error(JSON_ERROR_TYPE_MISMATCH);
    }

    // Could use a hashmap to get faster retrieval.
    for (u32_t i = 0; i < obj.value.object.count; i++) {
        if (re_str_cmp(key, obj.value.object.keys[i]) != 0) {
            continue;
        }
        return obj.value.object.values[i];
    }

    return json_error(JSON_ERROR_PROPERTY_NOT_FOUND);
}

json_object_t json_array(json_object_t obj, u32_t index) {
    if (obj.type != JSON_TYPE_ARRAY) {
        return json_error(JSON_ERROR_TYPE_MISMATCH);
    }

    if (index >= obj.value.array.count) {
        return json_error(JSON_ERROR_ARRAY_OUT_OF_BOUNDS);
    }

    return obj.value.array.values[index];
}

b8_t json_bool(json_object_t obj) {
    if (obj.type != JSON_TYPE_BOOL) {
        return false;
    }

    return obj.value.bool;
}

json_object_t json_path(json_object_t obj, re_str_t path) {
    json_object_t final = obj;

    for (u32_t i = 0; path.str[i] != '\0';) {
        // Array index
        if (path.str[i] == '[') {
            u32_t start = i;
            for (; path.str[i] != ']'; i++);
            u32_t end = i;
            i++;
            re_str_t idx_str = re_str_sub(path, start, end - 1);
            u32_t idx = atoi((const char *) idx_str.str);
            final = json_array(final, idx);
            continue;
        }

        if (path.str[i] == '/') {
            i++;
        }

        u32_t start = i;
        for (; path.str[i] != '/' && path.str[i] != '[' && path.str[i] != '\0'; i++);
        u32_t end = i;

        re_str_t key = re_str_sub(path, start, end - 1);
        final = json_object(final, key);
    }

    return final;
}

json_object_t json_path(json_object_t obj, re_str_t path);
