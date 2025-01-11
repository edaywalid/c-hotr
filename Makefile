CC := gcc
CFLAGS := -Wall -Wextra -MMD -MP
LDFLAGS := -ldl -pthread

# Build type settings
ifdef DEBUG
	CFLAGS += -g -DDEBUG
else
	CFLAGS += -O2
endif

# Directories
BUILD_DIR := build
SRC_DIR := .
INCLUDE_DIR := .
LIB_DIR := .


# Source files
MAIN_SRC := $(SRC_DIR)/main.c $(SRC_DIR)/hotr.c
HOT_LIB_SRC := $(SRC_DIR)/example.c

# Object files
MAIN_OBJ := $(MAIN_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
HOT_LIB_OBJ := $(HOT_LIB_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Dependency files
DEPS := $(MAIN_OBJ:.o=.d) $(HOT_LIB_OBJ:.o=.d)

# Output files
MAIN_TARGET := $(BUILD_DIR)/main
HOT_LIB_TARGET := $(LIB_DIR)/libhot.so

# Include directories
INCLUDES := -I$(INCLUDE_DIR)

# Default target
all: dirs $(MAIN_TARGET) $(HOT_LIB_TARGET)

# Create necessary directories
dirs:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR)

# Build main executable
$(MAIN_TARGET): $(MAIN_OBJ)
	@echo "Building main executable..."
	$(CC) $(MAIN_OBJ) -o $@ $(LDFLAGS)

# Build hot reload library
$(HOT_LIB_TARGET): $(HOT_LIB_OBJ)
	@echo "Building hot reload library..."
	@mkdir -p $(dir $@)
	$(CC) -shared -fPIC $^ -o $@

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Quick rebuild of just the hot library
hot: $(HOT_LIB_TARGET)
	@echo "Hot library rebuilt"

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR) $(LIB_DIR)

# Clean and rebuild everything
rebuild: clean all

# Development target that sets up file watching
watch:
	@echo "Watching for changes in $(SRC_DIR)/your_code.c..."
	@while true; do \
		$(MAKE) hot; \
		inotifywait -e modify $(SRC_DIR)/your_code.c; \
	done

# Include dependency files
-include $(DEPS)

.PHONY: all dirs clean rebuild hot watch
