#include "hotr.h"
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  printf("Starting program...\n");
  HotReloader *reloader = hot_reload_init("./libhot.so");
  if (!reloader) {
    fprintf(stderr, "Failed to initialize hot reloader\n");
    return 1;
  }

  void *handle = hot_reload_update(reloader);
  if (handle) {
    void (*func)(void) = dlsym(handle, "hello_world_func");
    if (func != NULL)
      func();
  }

  printf("Watching for changes to libhot.so...\n");

  while (1) {
    fflush(stdout);
    if (watch_for_changes(reloader) > 0 && needs_reload(reloader)) {
      printf("Detected change, reloading...\n");
      void *handle = hot_reload_update(reloader);
      if (handle) {
        dlerror();
        void (*func)(void) = dlsym(handle, "hello_world_func");
        if (func) {
          func();
          sleep(1);
        }
      }
    }
  }

  hot_reload_cleanup(reloader);
  return 0;
}
