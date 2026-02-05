#include "../include/ui.h"
#include "../include/core.h"
#include <stdio.h>
#include <string.h>

// ANSI color codes
#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define GREY    "\x1b[90m"

static const char* style_condition_color(int code) {
    if (code == 0 || code == 1) return YELLOW;
    if (code >= 2 && code <= 48) return GREY;
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return BLUE;
    if ((code >= 71 && code <= 77) || (code >= 85 && code <= 86)) return WHITE;
    if (code >= 95 && code <= 99) return MAGENTA;
    return RESET;
}

static const char* style_temp_color(double temp, int is_max) {
    if (is_max) {
        if (temp > 30.0) return RED;
        if (temp > 20.0) return YELLOW;
        return GREEN;
    } else {
        if (temp < 0.0) return BLUE;
        if (temp < 10.0) return CYAN;
        return GREEN;
    }
}

void print_forecast(const Forecast* forecast) {
    printf("%sFound location:%s %s%s, %s%s\n",
           BOLD, RESET, GREEN, forecast->location.name,
           forecast->location.country, RESET);
    printf("Fetching forecast... %sDone.%s\n\n", GREEN, RESET);
    
    printf("%s7-Day Forecast:%s\n", BOLD, RESET);
    
    // Print header
    printf("%s%-12s%s | %s%-25s%s | %s%-22s%s | %s%-22s%s\n",
           BOLD, "Date", RESET,
           BOLD, "Condition", RESET,
           BOLD, "Max Temp", RESET,
           BOLD, "Min Temp", RESET);
    
    printf("-------------+---------------------------+------------------------+------------------------\n");
    
    // Print each day
    for (size_t i = 0; i < forecast->daily_count; i++) {
        const DailyForecast* day = &forecast->daily[i];
        
        Fahrenheit max_f = celsius_to_fahrenheit(day->max_temp);
        Fahrenheit min_f = celsius_to_fahrenheit(day->min_temp);
        
        const char* condition = get_weather_description(day->weather_code);
        const char* cond_color = style_condition_color(day->weather_code);
        const char* max_color = style_temp_color(day->max_temp.value, 1);
        const char* min_color = style_temp_color(day->min_temp.value, 0);
        
        printf("%-12s | %s%-25s%s | %s%-5.1f°C / %-5.1f°F%s | %s%-5.1f°C / %-5.1f°F%s\n",
               day->date,
               cond_color, condition, RESET,
               max_color, day->max_temp.value, max_f.value, RESET,
               min_color, day->min_temp.value, min_f.value, RESET);
    }
    
    printf("\n");
}
