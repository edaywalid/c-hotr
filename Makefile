CC := gcc
LDFLAGS := -ldl

# Output files
MAIN_TARGET := main
HOT_LIB_TARGET := libhot.so

# Default target
all: $(HOT_LIB_TARGET) $(MAIN_TARGET) RUN_MAIN

# Build main executable
$(MAIN_TARGET): main.c hotr.c
	$(CC) main.c hotr.c -o $(MAIN_TARGET) $(LDFLAGS)

# Build hot reload library
$(HOT_LIB_TARGET): example.c
	$(CC) -shared -fPIC example.c -o $(HOT_LIB_TARGET)


RUN_MAIN:
	./$(MAIN_TARGET)

# Quick rebuild of just the hot library
hot: $(HOT_LIB_TARGET)
	@echo "Hot library rebuilt"

# Clean build files
clean:
	rm -f $(MAIN_TARGET) $(HOT_LIB_TARGET)

# Clean and rebuild everything
rebuild: clean all

.PHONY: all clean rebuild hot
