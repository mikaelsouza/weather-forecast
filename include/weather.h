#ifndef WEATHER_H
#define WEATHER_H

#include "core.h"

typedef struct {
    Latitude lat;
    Longitude lon;
    char* name;
    char* country;
} Location;

typedef struct {
    char* date;
    int weather_code;
    Celsius max_temp;
    Celsius min_temp;
} DailyForecast;

typedef struct {
    Location location;
    DailyForecast* daily;
    size_t daily_count;
} Forecast;

// Find location by city name
Location* find_location(const char* city);

// Get forecast for location
Forecast* get_forecast(Location* location);

// Get weather description from code
const char* get_weather_description(int code);

// Free functions
void location_free(Location* loc);
void forecast_free(Forecast* fc);

#endif // WEATHER_H
