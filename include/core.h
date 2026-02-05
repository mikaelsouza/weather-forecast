#ifndef CORE_H
#define CORE_H

#include <stddef.h>

// Type definitions
typedef struct {
    double value;
} Latitude;

typedef struct {
    double value;
} Longitude;

typedef struct {
    double value;
} Celsius;

typedef struct {
    double value;
} Fahrenheit;

// Temperature conversion
Fahrenheit celsius_to_fahrenheit(Celsius c);

// String utilities
char* strdup_safe(const char* s);
void* malloc_safe(size_t size);

#endif // CORE_H
