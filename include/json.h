#ifndef JSON_H
#define JSON_H

#include <stddef.h>

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonValue JsonValue;
typedef struct JsonObject JsonObject;
typedef struct JsonArray JsonArray;

struct JsonValue {
    JsonType type;
    union {
        int bool_val;
        double number_val;
        char* string_val;
        JsonArray* array_val;
        JsonObject* object_val;
    } data;
};

struct JsonArray {
    JsonValue** items;
    size_t count;
};

struct JsonObject {
    char** keys;
    JsonValue** values;
    size_t count;
};

// Parse JSON string
JsonValue* json_parse(const char* json_str);

// Free JSON value
void json_free(JsonValue* val);

// Accessors
JsonValue* json_object_get(JsonValue* obj, const char* key);
double json_as_number(JsonValue* val, double default_val);
const char* json_as_string(JsonValue* val, const char* default_val);
JsonArray* json_as_array(JsonValue* val);

#endif // JSON_H
