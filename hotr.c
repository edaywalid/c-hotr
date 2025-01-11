#include "hotr.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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
