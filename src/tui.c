#include "../include/tui.h"
#include "../include/weather.h"
#include "../include/ui.h"
#include "../include/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

// ANSI Escape Codes
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME  "\033[H"
#define HIDE_CURSOR  "\033[?25l"
#define SHOW_CURSOR  "\033[?25h"
#define COLOR_RESET  "\033[0m"
#define BOLD         "\033[1m"
#define BOX_COLOR    "\033[38;5;244m" // Gray

static struct termios orig_termios;

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf(SHOW_CURSOR);
}

static void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL);
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf(HIDE_CURSOR);
}

static void draw_box(int x, int y, int w, int h, const char* title) {
    printf(BOX_COLOR);
    // Top
    printf("\033[%d;%dH┌", y, x);
    for (int i = 0; i < w - 2; i++) printf("─");
    printf("┐");
    
    // Bottom
    printf("\033[%d;%dH└", y + h - 1, x);
    for (int i = 0; i < w - 2; i++) printf("─");
    printf("┘");
    
    // Sides
    for (int i = 1; i < h - 1; i++) {
        printf("\033[%d;%dH│", y + i, x);
        printf("\033[%d;%dH│", y + i, x + w - 1);
    }
    
    if (title) {
        printf("\033[%d;%dH " BOLD "%s" COLOR_RESET BOX_COLOR " ", y, x + 2, title);
    }
    printf(COLOR_RESET);
}

static void render_dashboard(const char* search_query, Forecast* forecast, const char* city_name) {
    printf(CLEAR_SCREEN CURSOR_HOME);
    
    // Logo / Title
    printf("\033[2;4H" UI_BOLD UI_YELLOW "🌤️  SUPREME WEATHER CLI " UI_RESET);
    
    // Search Box
    draw_box(4, 4, 60, 3, " Search City ");
    printf("\033[5;6H" UI_CYAN "> " UI_RESET "%s_", search_query);
    
    // Results Box
    draw_box(4, 8, 70, 15, city_name ? city_name : " Results ");
    
    if (forecast) {
        printf("\033[10;6H" UI_BOLD "Date         | Condition                 | Max Temp / Min Temp" UI_RESET);
        printf("\033[11;6H-------------+---------------------------+------------------------");
        for (int i = 0; i < 7; i++) {
            const DailyForecast* day = &forecast->daily[i];
            const char* cond_color = style_condition_color(day->weather_code);
            const char* max_color = style_temp_color(day->max_temp.value, 1);
            const char* min_color = style_temp_color(day->min_temp.value, 0);
            
            printf("\033[%d;6H%-12s | %s%-25s%s | %s%.1f °C%s / %s%.1f °C%s", 
                   12 + i, 
                   day->date,
                   cond_color, get_weather_description(day->weather_code), UI_RESET,
                   max_color, day->max_temp.value, UI_RESET,
                   min_color, day->min_temp.value, UI_RESET);
        }
    } else if (city_name && strcmp(city_name, "Loading...") == 0) {
        printf("\033[12;30H" UI_YELLOW "Fetching data..." UI_RESET);
    } else {
        printf("\033[12;25H" UI_BOLD "Type a city and press ENTER" UI_RESET);
        printf("\033[14;22H" BOX_COLOR "(Press ESC or 'q' to quit)" UI_RESET);
    }
    
    fflush(stdout);
}

void launch_tui(void) {
    enable_raw_mode();
    
    char search_query[64] = "";
    int query_len = 0;
    Forecast* current_forecast = NULL;
    char* current_city = NULL;
    
    render_dashboard(search_query, NULL, NULL);
    
    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) continue;
        
        if (c == 27) { // ESC or arrow key
            char next;
            if (read(STDIN_FILENO, &next, 2) <= 0) break; // Pure ESC
            continue; // Ignore arrow keys for now
        }
        
        if (c == 'q' && query_len == 0) break;
        
        if (c == 13) { // ENTER
            if (query_len > 0) {
                render_dashboard(search_query, NULL, "Loading...");
                
                Location* loc = find_location(search_query);
                if (loc) {
                    if (current_forecast) forecast_free(current_forecast);
                    current_forecast = get_forecast(loc); // get_forecast(Location*)
                    
                    if (current_city) free(current_city);
                    current_city = strdup_safe(loc->name);
                    
                    location_free(loc);
                } else {
                    if (current_city) free(current_city);
                    current_city = strdup_safe("City not found");
                }
                
                query_len = 0;
                search_query[0] = '\0';
                render_dashboard(search_query, current_forecast, current_city);
            }
        } else if (c == 127 || c == 8) { // Backspace
            if (query_len > 0) {
                search_query[--query_len] = '\0';
                render_dashboard(search_query, current_forecast, current_city);
            }
        } else if (c >= 32 && c <= 126) { // Printable chars
            if (query_len < 60) {
                search_query[query_len++] = c;
                search_query[query_len] = '\0';
                render_dashboard(search_query, current_forecast, current_city);
            }
        }
    }
    
    if (current_forecast) forecast_free(current_forecast);
    if (current_city) free(current_city);
    printf(CLEAR_SCREEN CURSOR_HOME SHOW_CURSOR);
}
