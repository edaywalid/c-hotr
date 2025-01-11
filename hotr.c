#include "hotr.h"
#include <stdlib.h>
#include <string.h>

HotReloader *hot_reload_init(const char *lib_path) {
  HotReloader *reloader = malloc(sizeof(HotReloader));

  reloader->lib_path = strdup(lib_path);
  reloader->handle = NULL;
  reloader->last_modified = 0;
  reloader->cleanup = NULL;

  return reloader;
}
