#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/json.h"
#include "../include/core.h"
#include "../include/weather.h"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running %s...\n", #name); \
    test_##name(); \
    tests_run++; \
    tests_passed++; \
    printf("  ✓ PASSED\n"); \
} while(0)

// JSON Parser Tests
TEST(json_parse_null) {
    JsonValue* val = json_parse("null");
    assert(val != NULL);
    assert(val->type == JSON_NULL);
    json_free(val);
}

TEST(json_parse_bool) {
    JsonValue* val1 = json_parse("true");
    assert(val1 != NULL);
    assert(val1->type == JSON_BOOL);
    assert(val1->data.bool_val == 1);
    json_free(val1);
    
    JsonValue* val2 = json_parse("false");
    assert(val2 != NULL);
    assert(val2->type == JSON_BOOL);
    assert(val2->data.bool_val == 0);
    json_free(val2);
}

TEST(json_parse_number) {
    JsonValue* val = json_parse("123.45");
    assert(val != NULL);
    assert(val->type == JSON_NUMBER);
    assert(val->data.number_val == 123.45);
    json_free(val);
}

TEST(json_parse_string) {
    JsonValue* val = json_parse("\"hello world\"");
    assert(val != NULL);
    assert(val->type == JSON_STRING);
    assert(strcmp(val->data.string_val, "hello world") == 0);
    json_free(val);
}

TEST(json_parse_array) {
    JsonValue* val = json_parse("[1, 2, 3]");
    assert(val != NULL);
    assert(val->type == JSON_ARRAY);
    JsonArray* arr = val->data.array_val;
    assert(arr->count == 3);
    assert(arr->items[0]->data.number_val == 1.0);
    assert(arr->items[1]->data.number_val == 2.0);
    assert(arr->items[2]->data.number_val == 3.0);
    json_free(val);
}

TEST(json_parse_object) {
    JsonValue* val = json_parse("{\"key\": \"value\", \"number\": 42}");
    assert(val != NULL);
    assert(val->type == JSON_OBJECT);
    
    JsonValue* key_val = json_object_get(val, "key");
    assert(key_val != NULL);
    assert(strcmp(json_as_string(key_val, ""), "value") == 0);
    
    JsonValue* num_val = json_object_get(val, "number");
    assert(num_val != NULL);
    assert(json_as_number(num_val, 0.0) == 42.0);
    
    json_free(val);
}

TEST(json_parse_nested) {
    const char* json = "{\"array\": [1, 2], \"obj\": {\"nested\": true}}";
    JsonValue* val = json_parse(json);
    assert(val != NULL);
    assert(val->type == JSON_OBJECT);
    
    JsonValue* arr = json_object_get(val, "array");
    assert(arr != NULL);
    assert(arr->type == JSON_ARRAY);
    
    JsonValue* obj = json_object_get(val, "obj");
    assert(obj != NULL);
    assert(obj->type == JSON_OBJECT);
    
    json_free(val);
}

// Core Tests
TEST(celsius_to_fahrenheit_conversion) {
    Celsius c = {0.0};
    Fahrenheit f = celsius_to_fahrenheit(c);
    assert(f.value == 32.0);
    
    Celsius c2 = {100.0};
    Fahrenheit f2 = celsius_to_fahrenheit(c2);
    assert(f2.value == 212.0);
}

TEST(weather_description_codes) {
    assert(strcmp(get_weather_description(0), "Clear sky") == 0);
    assert(strcmp(get_weather_description(3), "Overcast") == 0);
    assert(strcmp(get_weather_description(61), "Slight rain") == 0);
    assert(strcmp(get_weather_description(95), "Thunderstorm") == 0);
    assert(strcmp(get_weather_description(999), "Unknown") == 0);
}

int main(void) {
    printf("=== Pure C Weather CLI Test Suite ===\n\n");
    
    // JSON Tests
    printf("JSON Parser Tests:\n");
    RUN_TEST(json_parse_null);
    RUN_TEST(json_parse_bool);
    RUN_TEST(json_parse_number);
    RUN_TEST(json_parse_string);
    RUN_TEST(json_parse_array);
    RUN_TEST(json_parse_object);
    RUN_TEST(json_parse_nested);
    
    // Core Tests
    printf("\nCore Tests:\n");
    RUN_TEST(celsius_to_fahrenheit_conversion);
    RUN_TEST(weather_description_codes);
    
    printf("\n=== Test Results ===\n");
    printf("Total: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_run - tests_passed);
    
    if (tests_run == tests_passed) {
        printf("\n✓ ALL TESTS PASSED!\n");
        return 0;
    } else {
        printf("\n✗ SOME TESTS FAILED\n");
        return 1;
    }
}
