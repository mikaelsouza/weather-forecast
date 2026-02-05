#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/weather.h"
#include "../include/ui.h"

#define VERSION "1.0.0"

static void print_help(void) {
    printf("\nSupreme Weather Forecast CLI 🌤️  (Pure C Edition)\n\n");
    printf("USAGE:\n");
    printf("    weather-cli [CITY]\n");
    printf("    weather-cli --help\n");
    printf("    weather-cli --version\n\n");
    printf("If no CITY is provided, the application starts in interactive mode.\n\n");
    printf("FLAGS:\n");
    printf("    -h, --help       Prints help information\n");
    printf("    -v, --version    Prints version information\n\n");
}

static void interactive_mode(void) {
    printf("\x1b[1mWeather Forecast CLI v%s\x1b[0m\n", VERSION);
    printf("--------------------------------------\n");
    
    char input[256];
    while (1) {
        printf("\nEnter city name (or 'q' to quit): ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "q") == 0 || strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        
        if (strlen(input) == 0) {
            continue;
        }
        
        Location* loc = find_location(input);
        if (!loc) {
            fprintf(stderr, "\x1b[31mError:\x1b[0m Failed to find location\n");
            continue;
        }
        
        Forecast* fc = get_forecast(loc);
        if (!fc) {
            fprintf(stderr, "\x1b[31mError:\x1b[0m Failed to get forecast\n");
            location_free(loc);
            continue;
        }
        
        print_forecast(fc);
        
        forecast_free(fc);
        location_free(loc);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_help();
            return 0;
        }
        
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("weather-cli v%s\n", VERSION);
            return 0;
        }
        
        // Direct city lookup
        const char* city = argv[1];
        
        Location* loc = find_location(city);
        if (!loc) {
            fprintf(stderr, "\x1b[31mError:\x1b[0m Failed to find location\n");
            return 1;
        }
        
        Forecast* fc = get_forecast(loc);
        if (!fc) {
            fprintf(stderr, "\x1b[31mError:\x1b[0m Failed to get forecast\n");
            location_free(loc);
            return 1;
        }
        
        print_forecast(fc);
        
        forecast_free(fc);
        location_free(loc);
        
        return 0;
    }
    
    // No arguments - start interactive mode
    interactive_mode();
    
    return 0;
}
