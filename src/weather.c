#include "../include/weather.h"
#include "../include/http.h"
#include "../include/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GEOCODING_API "http://geocoding-api.open-meteo.com/v1/search"
#define FORECAST_API "http://api.open-meteo.com/v1/forecast"

static char* url_encode(const char* str) {
    size_t len = strlen(str);
    char* encoded = malloc_safe(len * 3 + 1);
    char* p = encoded;
    
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        } else {
            sprintf(p, "%%%02X", (unsigned char)c);
            p += 3;
        }
    }
    *p = '\0';
    return encoded;
}

Location* find_location(const char* city) {
    char* encoded_city = url_encode(city);
    char url[512];
    snprintf(url, sizeof(url), "%s?name=%s&count=1&language=en&format=json",
             GEOCODING_API, encoded_city);
    free(encoded_city);
    
    char* response = http_get(url);
    if (!response) {
        fprintf(stderr, "Failed to fetch location\n");
        return NULL;
    }
    
    JsonValue* json = json_parse(response);
    free(response);
    
    if (!json) {
        fprintf(stderr, "Failed to parse JSON\n");
        return NULL;
    }
    
    JsonValue* results = json_object_get(json, "results");
    JsonArray* arr = json_as_array(results);
    
    if (!arr || arr->count == 0) {
        fprintf(stderr, "Location not found: %s\n", city);
        json_free(json);
        return NULL;
    }
    
    JsonValue* first = arr->items[0];
    
    Location* loc = malloc_safe(sizeof(Location));
    loc->lat.value = json_as_number(json_object_get(first, "latitude"), 0.0);
    loc->lon.value = json_as_number(json_object_get(first, "longitude"), 0.0);
    loc->name = strdup_safe(json_as_string(json_object_get(first, "name"), "Unknown"));
    loc->country = strdup_safe(json_as_string(json_object_get(first, "country"), "Unknown"));
    
    json_free(json);
    return loc;
}

Forecast* get_forecast(Location* location) {
    char url[512];
    snprintf(url, sizeof(url),
             "%s?latitude=%.4f&longitude=%.4f&daily=weathercode,temperature_2m_max,temperature_2m_min&timezone=auto",
             FORECAST_API, location->lat.value, location->lon.value);
    
    char* response = http_get(url);
    if (!response) {
        fprintf(stderr, "Failed to fetch forecast\n");
        return NULL;
    }
    
    JsonValue* json = json_parse(response);
    free(response);
    
    if (!json) {
        fprintf(stderr, "Failed to parse forecast JSON\n");
        return NULL;
    }
    
    JsonValue* daily_obj = json_object_get(json, "daily");
    if (!daily_obj) {
        fprintf(stderr, "No 'daily' field in forecast response. Available keys:\n");
        if (json->type == JSON_OBJECT) {
            for (size_t i = 0; i < json->data.object_val->count; i++) {
                fprintf(stderr, "  - %s\n", json->data.object_val->keys[i]);
            }
        }
        json_free(json);
        return NULL;
    }
    
    JsonArray* times = json_as_array(json_object_get(daily_obj, "time"));
    JsonArray* codes = json_as_array(json_object_get(daily_obj, "weather_code"));
    if (!codes) {
        codes = json_as_array(json_object_get(daily_obj, "weathercode"));
    }
    JsonArray* max_temps = json_as_array(json_object_get(daily_obj, "temperature_2m_max"));
    JsonArray* min_temps = json_as_array(json_object_get(daily_obj, "temperature_2m_min"));
    
    if (!times || !codes || !max_temps || !min_temps) {
        fprintf(stderr, "Missing forecast arrays\n");
        json_free(json);
        return NULL;
    }
    
    Forecast* fc = malloc_safe(sizeof(Forecast));
    fc->location = *location;
    fc->location.name = strdup_safe(location->name);
    fc->location.country = strdup_safe(location->country);
    fc->daily_count = times->count;
    fc->daily = malloc_safe(fc->daily_count * sizeof(DailyForecast));
    
    for (size_t i = 0; i < fc->daily_count; i++) {
        fc->daily[i].date = strdup_safe(json_as_string(times->items[i], "N/A"));
        fc->daily[i].weather_code = (int)json_as_number(codes->items[i], -1);
        fc->daily[i].max_temp.value = json_as_number(max_temps->items[i], 0.0);
        fc->daily[i].min_temp.value = json_as_number(min_temps->items[i], 0.0);
    }
    
    json_free(json);
    return fc;
}

const char* get_weather_description(int code) {
    switch (code) {
        case 0: return "Clear sky";
        case 1: return "Mainly clear";
        case 2: return "Partly cloudy";
        case 3: return "Overcast";
        case 45: return "Fog";
        case 48: return "Depositing rime fog";
        case 51: return "Light drizzle";
        case 53: return "Moderate drizzle";
        case 55: return "Dense drizzle";
        case 56: return "Light freezing drizzle";
        case 57: return "Dense freezing drizzle";
        case 61: return "Slight rain";
        case 63: return "Moderate rain";
        case 65: return "Heavy rain";
        case 66: return "Light freezing rain";
        case 67: return "Heavy freezing rain";
        case 71: return "Slight snow fall";
        case 73: return "Moderate snow fall";
        case 75: return "Heavy snow fall";
        case 77: return "Snow grains";
        case 80: return "Slight rain showers";
        case 81: return "Moderate rain showers";
        case 82: return "Violent rain showers";
        case 85: return "Slight snow showers";
        case 86: return "Heavy snow showers";
        case 95: return "Thunderstorm";
        case 96: return "Thunderstorm with hail";
        case 99: return "Heavy thunderstorm w/ hail";
        default: return "Unknown";
    }
}

void location_free(Location* loc) {
    if (!loc) return;
    free(loc->name);
    free(loc->country);
    free(loc);
}

void forecast_free(Forecast* fc) {
    if (!fc) return;
    free(fc->location.name);
    free(fc->location.country);
    for (size_t i = 0; i < fc->daily_count; i++) {
        free(fc->daily[i].date);
    }
    free(fc->daily);
    free(fc);
}
