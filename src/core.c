#include "../include/core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Fahrenheit celsius_to_fahrenheit(Celsius c) {
    Fahrenheit f;
    f.value = c.value * 9.0 / 5.0 + 32.0;
    return f;
}

char* strdup_safe(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

void* malloc_safe(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Fatal: malloc failed\n");
        exit(1);
    }
    return ptr;
}
