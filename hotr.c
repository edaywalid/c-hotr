#include "hotr.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

HotReloader *hot_reload_init(const char *lib_path) {
  HotReloader *reloader = malloc(sizeof(HotReloader));

  reloader->lib_path = strdup(lib_path);
  reloader->handle = NULL;
  reloader->last_modified = 0;
  reloader->cleanup = NULL;

  return reloader;
}

int needs_reload(HotReloader *reloader) {
  struct stat attr;
  if (stat(reloader->lib_path, &attr) == 0) {
    if (attr.st_mtime > reloader->last_modified) {
      return 1;
    }
  }
  return 0;
}

/*
 * Load or reload the library
 * 1 - Call cleanup function if it exists
 * 2 - Close existing library
 * 3 - Load new library
 * 4 - Update last time modified
 * 5 - Get cleanup function
 */
void *hot_reload_update(HotReloader *reloader) {
  if (!needs_reload(reloader)) {
    return reloader->handle;
  }

  if (reloader->cleanup != NULL) {
    reloader->cleanup();
  }

  if (reloader->handle != NULL) {
    dlclose(reloader->handle);
  }

  reloader->handle = dlopen(reloader->lib_path, RTLD_NOW);
  if (!reloader->handle) {
    fprintf(stderr, "Error loading library: %s\n", dlerror());
    return NULL;
  }

  struct stat attr;
  stat(reloader->lib_path, &attr);
  reloader->last_modified = attr.st_mtime;

  reloader->cleanup = dlsym(reloader->handle, "cleanup");

  return reloader->handle;
}

void hot_reload_cleanup(HotReloader *reloader) {
  if (reloader->cleanup != NULL) {
    reloader->cleanup();
  }
  if (reloader->handle != NULL) {
    dlclose(reloader->handle);
  }
  free(reloader->lib_path);
  free(reloader);
}
