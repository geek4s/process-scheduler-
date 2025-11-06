# ==============================================================
# Process Scheduler - Makefile
# Builds shared library libprocess.so for Python GUI integration
# ==============================================================

# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -fPIC
LDFLAGS = -shared

# Source files
SRCS = process_manager.c statistics_manager.c process_lib.c
OBJS = $(SRCS:.c=.o)

# Output shared library
TARGET = ../lib/libprocess.so

# Default rule
all: $(TARGET)

# Build shared library
$(TARGET): $(OBJS)
	@mkdir -p ../lib
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	@echo "✅ Built: $(TARGET)"

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run standalone test (optional)
# Example: make test
test: all
	@echo "\n--- Running Library Test ---"
	$(CC) process_manager.c statistics_manager.c process_lib.c -DTEST_LIB -o test_lib
	./test_lib

# Clean build files
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f test_lib
	@echo "🧹 Cleaned build artifacts."

# Phony targets (not actual files)
.PHONY: all clean test
