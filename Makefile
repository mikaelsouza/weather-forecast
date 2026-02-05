CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS =

SRCS = src/core.c src/http.c src/json.c src/weather.c src/ui.c src/tui.c src/main.c
TARGET = weather-c

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) test-suite weather-c-final
	rm -rf *.dSYM

test: test.c src/core.c src/json.c src/weather.c src/http.c src/tui.c
	$(CC) $(CFLAGS) test.c src/core.c src/json.c src/weather.c src/http.c src/tui.c -o test-suite
	./test-suite

install: $(TARGET)
	@echo "Binary built: ./$(TARGET)"
	@echo "To install: sudo cp $(TARGET) /usr/local/bin/"

.DEFAULT_GOAL := all
