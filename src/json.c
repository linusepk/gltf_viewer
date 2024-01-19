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

static void skip_whitespace(parser_t *parser) {
    while ((peek(*parser, 0) == ' ' ||
            peek(*parser, 0) == '\n' ||
            peek(*parser, 0) == '\r' ||
            peek(*parser, 0) == '\t') &&
            peek(*parser, 0) != '\0' &&
            parser->i < parser->buffer.len) {
        if (peek(*parser, 0) == '\n') {
            parser->x = 0;
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

    return (json_object_t) {JSON_TYPE_STRING, {re_str_sub(parser->buffer, start, end)}};
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
            return parse_number(parser);
    }

    return (json_object_t) {0};
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
        }
        obj.value.object.keys[obj_i] = parse_string(parser).value.string;

        skip_whitespace(parser);
        if (peek(*parser, 0) != ':') {
            re_log_debug("%c", peek(*parser, 0));
            re_log_debug("%u:%u: Missing colon after key.", parser->y, parser->x); 
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
        }
    }
    skip(parser, 1);

    return obj;
}

json_object_t json_parse(re_str_t data) {
    parser_t parser = {0};
    parser.buffer = data;

    skip_whitespace(&parser);

    switch (peek(parser, 0)) {
        case '{':
            return parse_object(&parser);
        case '[':
            re_log_debug("Arrays not implemented.");
            break;
        default:
            re_log_error("Invalid starting json.");
            break;
    }

    return (json_object_t) {0};
}
