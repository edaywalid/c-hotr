# C Hot Reload Library

A lightweight library for implementing hot reloading functionality in C applications. This library allows you to modify and reload shared libraries at runtime without restarting your application.

## Features

- File system event-based monitoring (using inotify)
- Automatic cleanup handling
- Thread-safe implementation
- Non-blocking file change detection
- Proper resource management
- Error handling and reporting

## Prerequisites

- Linux operating system (uses inotify)
- GCC compiler
- Basic understanding of shared libraries in C

## Project Structure

```
.
├── hotr.h          # Header file with library interface
├── hotr.c          # Implementation of hot reload functionality
├── example.c       # Example shared library to be hot reloaded
└── main.c          # Example usage of the hot reload library
```

## Building

Compile the shared library that will be hot reloaded:
```bash
gcc -shared -fPIC example.c -o libhot.so
```

Compile the main program:
```bash
gcc main.c hotr.c -o main -ldl
```

## Usage

### 1. Create a shared library

Create a C file that will be compiled as a shared library. Functions in this file can be modified and reloaded at runtime.

```c
// example.c
#include <stdio.h>

void cleanup(void) {
    printf("cleanup\n");
    fflush(stdout);
}

void hello_world_func(void) {
    printf("hello world!\n");
    fflush(stdout);
}
```

### 2. Use the hot reload library in your main program

```c
// main.c
#include "hotr.h"
#include <stdio.h>
#include <dlfcn.h>

int main() {
    // Initialize hot reloader
    HotReloader *reloader = hot_reload_init("./libhot.so");
    if (!reloader) {
        fprintf(stderr, "Failed to initialize hot reloader\n");
        return 1;
    }

    // Initial load
    void *handle = hot_reload_update(reloader);
    if (handle) {
        void (*func)(void) = dlsym(handle, "hello_world_func");
        if (func) func();
    }

    // Watch for changes
    while (1) {
        if (watch_for_changes(reloader) > 0) {
            printf("Detected change, reloading...\n");
            void *handle = hot_reload_update(reloader);
            if (handle) {
                void (*func)(void) = dlsym(handle, "hello_world_func");
                if (func) func();
            }
        }
    }

    hot_reload_cleanup(reloader);
    return 0;
}
```

### 3. Run and test

1. Start the program:
```bash
./main
```

2. Modify the shared library source (example.c)
3. Recompile the shared library:
```bash
gcc -shared -fPIC example.c -o libhot.so
```

The program will automatically detect the change and reload the library , we will add in the future an automatic builder when the selected shared library code is changed

## API Reference

### HotReloader

Main structure for managing hot reloading:

```c
typedef struct {
    char *lib_path;           // Path to the shared library
    void *handle;            // Handle to the loaded library
    time_t last_modified;    // Last modification time
    void (*cleanup)(void);   // Cleanup function pointer
    int inotify_fd;         // File descriptor for inotify
    int watch_fd;           // Watch descriptor
} HotReloader;
```

### Functions

- `HotReloader *hot_reload_init(const char *lib_path)`
  - Initializes the hot reload system for the specified library
  - Returns NULL on failure

- `void *hot_reload_update(HotReloader *reloader)`
  - Reloads the library if changes are detected
  - Returns handle to the loaded library or NULL on failure

- `int watch_for_changes(HotReloader *reloader)`
  - Blocks until changes are detected in the library file
  - Returns 1 if changes detected, 0 if no changes, -1 on error

- `void hot_reload_cleanup(HotReloader *reloader)`
  - Cleans up resources used by the hot reload system

## Limitations

- Linux-only (uses inotify)
- Library must maintain ABI compatibility between reloads
- Cleanup function must be implemented properly to prevent memory leaks
- Not suitable for security-critical applications

## Best Practices

1. Always implement a cleanup function in your shared library
2. Use proper error handling in your main program
3. Be careful with global state in the shared library
4. Ensure ABI compatibility when modifying the shared library
5. Add proper flushing (fflush) after printf calls in the shared library


