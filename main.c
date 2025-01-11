#include "hotr.h"
#include <dlfcn.h>
#include <unistd.h>

int main() {
  HotReloader *reloader = hot_reload_init("./libhot.so");

  while (1) {
    void *handle = hot_reload_update(reloader);
    if (handle) {
      void (*func)(void) = dlsym(handle, "hello_world_func");
      if (func) {
        func();
      }
    }

    sleep(1);
  }

  hot_reload_cleanup(reloader);
  return 0;
}
