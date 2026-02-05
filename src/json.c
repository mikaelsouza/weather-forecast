#include "../include/json.h"
#include "../include/core.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static const char* skip_whitespace(const char* p) {
    while (*p && isspace(*p)) p++;
    return p;
}

static JsonValue* parse_value(const char** p);

static JsonValue* parse_null(const char** p) {
    if (strncmp(*p, "null", 4) != 0) return NULL;
    *p += 4;
    JsonValue* val = malloc_safe(sizeof(JsonValue));
    val->type = JSON_NULL;
    return val;
}

static JsonValue* parse_bool(const char** p) {
    JsonValue* val = malloc_safe(sizeof(JsonValue));
    val->type = JSON_BOOL;
    if (strncmp(*p, "true", 4) == 0) {
        val->data.bool_val = 1;
        *p += 4;
    } else if (strncmp(*p, "false", 5) == 0) {
        val->data.bool_val = 0;
        *p += 5;
    } else {
        free(val);
        return NULL;
    }
    return val;
}

static JsonValue* parse_number(const char** p) {
    char* end;
    double num = strtod(*p, &end);
    if (end == *p) return NULL;
    
    *p = end;
    JsonValue* val = malloc_safe(sizeof(JsonValue));
    val->type = JSON_NUMBER;
    val->data.number_val = num;
    return val;
}

static JsonValue* parse_string(const char** p) {
    if (**p != '"') return NULL;
    (*p)++;
    
    const char* start = *p;
    size_t len = 0;
    while (**p && **p != '"') {
        if (**p == '\\') (*p)++;
        (*p)++;
        len++;
    }
    
    if (**p != '"') return NULL;
    
    char* str = malloc_safe(len + 1);
    const char* src = start;
    char* dst = str;
    
    while (src < *p) {
        if (*src == '\\') {
            src++;
            switch (*src) {
                case 'n': *dst = '\n'; break;
                case 't': *dst = '\t'; break;
                case 'r': *dst = '\r'; break;
                case '\\': *dst = '\\'; break;
                case '"': *dst = '"'; break;
                default: *dst = *src;
            }
        } else {
            *dst = *src;
        }
        dst++;
        src++;
    }
    *dst = '\0';
    
    (*p)++;
    
    JsonValue* val = malloc_safe(sizeof(JsonValue));
    val->type = JSON_STRING;
    val->data.string_val = str;
    return val;
}

static JsonValue* parse_array(const char** p) {
    if (**p != '[') return NULL;
    (*p)++;
    
    JsonArray* arr = malloc_safe(sizeof(JsonArray));
    arr->items = NULL;
    arr->count = 0;
    
    *p = skip_whitespace(*p);
    if (**p == ']') {
        (*p)++;
        JsonValue* val = malloc_safe(sizeof(JsonValue));
        val->type = JSON_ARRAY;
        val->data.array_val = arr;
        return val;
    }
    
    while (1) {
        *p = skip_whitespace(*p);
        JsonValue* item = parse_value(p);
        if (!item) {
            // Cleanup
            for (size_t i = 0; i < arr->count; i++) {
                json_free(arr->items[i]);
            }
            free(arr->items);
            free(arr);
            return NULL;
        }
        
        arr->items = realloc(arr->items, (arr->count + 1) * sizeof(JsonValue*));
        arr->items[arr->count++] = item;
        
        *p = skip_whitespace(*p);
        if (**p == ']') {
            (*p)++;
            break;
        }
        if (**p != ',') {
            // Cleanup
            for (size_t i = 0; i < arr->count; i++) {
                json_free(arr->items[i]);
            }
            free(arr->items);
            free(arr);
            return NULL;
        }
        (*p)++;
    }
    
    JsonValue* val = malloc_safe(sizeof(JsonValue));
    val->type = JSON_ARRAY;
    val->data.array_val = arr;
    return val;
}

static JsonValue* parse_object(const char** p) {
    if (**p != '{') return NULL;
    (*p)++;
    
    JsonObject* obj = malloc_safe(sizeof(JsonObject));
    obj->keys = NULL;
    obj->values = NULL;
    obj->count = 0;
    
    *p = skip_whitespace(*p);
    if (**p == '}') {
        (*p)++;
        JsonValue* val = malloc_safe(sizeof(JsonValue));
        val->type = JSON_OBJECT;
        val->data.object_val = obj;
        return val;
    }
    
    while (1) {
        *p = skip_whitespace(*p);
        JsonValue* key_val = parse_string(p);
        if (!key_val) {
            // Cleanup
            for (size_t i = 0; i < obj->count; i++) {
                free(obj->keys[i]);
                json_free(obj->values[i]);
            }
            free(obj->keys);
            free(obj->values);
            free(obj);
            return NULL;
        }
        
        char* key = key_val->data.string_val;
        free(key_val);
        
        *p = skip_whitespace(*p);
        if (**p != ':') {
            free(key);
            // Cleanup
            for (size_t i = 0; i < obj->count; i++) {
                free(obj->keys[i]);
                json_free(obj->values[i]);
            }
            free(obj->keys);
            free(obj->values);
            free(obj);
            return NULL;
        }
        (*p)++;
        
        *p = skip_whitespace(*p);
        JsonValue* value = parse_value(p);
        if (!value) {
            free(key);
            // Cleanup
            for (size_t i = 0; i < obj->count; i++) {
                free(obj->keys[i]);
                json_free(obj->values[i]);
            }
            free(obj->keys);
            free(obj->values);
            free(obj);
            return NULL;
        }
        
        obj->keys = realloc(obj->keys, (obj->count + 1) * sizeof(char*));
        obj->values = realloc(obj->values, (obj->count + 1) * sizeof(JsonValue*));
        obj->keys[obj->count] = key;
        obj->values[obj->count] = value;
        obj->count++;
        
        *p = skip_whitespace(*p);
        if (**p == '}') {
            (*p)++;
            break;
        }
        if (**p != ',') {
            // Cleanup
            for (size_t i = 0; i < obj->count; i++) {
                free(obj->keys[i]);
                json_free(obj->values[i]);
            }
            free(obj->keys);
            free(obj->values);
            free(obj);
            return NULL;
        }
        (*p)++;
    }
    
    JsonValue* val = malloc_safe(sizeof(JsonValue));
    val->type = JSON_OBJECT;
    val->data.object_val = obj;
    return val;
}

static JsonValue* parse_value(const char** p) {
    *p = skip_whitespace(*p);
    
    if (**p == 'n') return parse_null(p);
    if (**p == 't' || **p == 'f') return parse_bool(p);
    if (**p == '"') return parse_string(p);
    if (**p == '[') return parse_array(p);
    if (**p == '{') return parse_object(p);
    if (**p == '-' || isdigit(**p)) return parse_number(p);
    
    return NULL;
}

JsonValue* json_parse(const char* json_str) {
    const char* p = json_str;
    return parse_value(&p);
}

void json_free(JsonValue* val) {
    if (!val) return;
    
    switch (val->type) {
        case JSON_STRING:
            free(val->data.string_val);
            break;
        case JSON_ARRAY:
            if (val->data.array_val) {
                for (size_t i = 0; i < val->data.array_val->count; i++) {
                    json_free(val->data.array_val->items[i]);
                }
                free(val->data.array_val->items);
                free(val->data.array_val);
            }
            break;
        case JSON_OBJECT:
            if (val->data.object_val) {
                for (size_t i = 0; i < val->data.object_val->count; i++) {
                    free(val->data.object_val->keys[i]);
                    json_free(val->data.object_val->values[i]);
                }
                free(val->data.object_val->keys);
                free(val->data.object_val->values);
                free(val->data.object_val);
            }
            break;
        default:
            break;
    }
    
    free(val);
}

JsonValue* json_object_get(JsonValue* obj, const char* key) {
    if (!obj || obj->type != JSON_OBJECT) return NULL;
    
    JsonObject* o = obj->data.object_val;
    for (size_t i = 0; i < o->count; i++) {
        if (strcmp(o->keys[i], key) == 0) {
            return o->values[i];
        }
    }
    return NULL;
}

double json_as_number(JsonValue* val, double default_val) {
    if (!val || val->type != JSON_NUMBER) return default_val;
    return val->data.number_val;
}

const char* json_as_string(JsonValue* val, const char* default_val) {
    if (!val || val->type != JSON_STRING) return default_val;
    return val->data.string_val;
}

JsonArray* json_as_array(JsonValue* val) {
    if (!val || val->type != JSON_ARRAY) return NULL;
    return val->data.array_val;
}
