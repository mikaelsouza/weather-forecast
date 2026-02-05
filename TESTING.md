# Pure C Weather CLI - Test Suite

## Running Tests

```bash
make test
```

## Test Coverage

The test suite (`test.c`) includes:

### JSON Parser Tests (7 tests)
- `json_parse_null` - NULL value parsing
- `json_parse_bool` - Boolean parsing (true/false)
- `json_parse_number` - Numeric parsing
- `json_parse_string` - String parsing with escapes
- `json_parse_array` - Array parsing
- `json_parse_object` - Object parsing with key-value pairs
- `json_parse_nested` - Nested structures

### Core Tests (2 tests)
- `celsius_to_fahrenheit_conversion` - Temperature conversion logic
- `weather_description_codes` - WMO weather code mapping

## Manual Testing

```bash
# Interactive mode
./weather-c

# Direct query
./weather-c "Paris"
./weather-c "New York"
./weather-c "Tokyo"

# Help and version
./weather-c --help
./weather-c --version
```

## Test Framework

Zero dependencies - custom assertion-based testing with:
- RUN_TEST macro for automatic test execution
- Pass/fail tracking
- Clear output formatting
