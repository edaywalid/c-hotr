#ifndef HOT_RELOAD_H
#define HOT_RELOAD_H
#include <time.h>

typedef struct {
  char *lib_path;
  void *handle;
  time_t last_modified;
  void (*cleanup)(void);
} HotReloader;

#endif
