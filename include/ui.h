#ifndef UI_H
#define UI_H

#include "weather.h"

// Print forecast to stdout
// Print forecast to stdout
void print_forecast(const Forecast* forecast);

// Color styling helpers
const char* style_condition_color(int code);
const char* style_temp_color(double temp, int is_max);

// ANSI color codes (exposed for TUI)
#define UI_RESET   "\x1b[0m"
#define UI_BOLD    "\x1b[1m"
#define UI_GREEN   "\x1b[32m"
#define UI_YELLOW  "\x1b[33m"
#define UI_BLUE    "\x1b[34m"
#define UI_CYAN    "\x1b[36m"
#define UI_RESET   "\x1b[0m"

#endif // UI_H
