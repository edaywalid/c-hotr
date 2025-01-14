#ifndef HOT_RELOAD_H
#define HOT_RELOAD_H

#include <time.h>

typedef struct {
  char *lib_path;
  void *handle;
  time_t last_modified;
  void (*cleanup)(void);
  int inotify_fd;
  int watch_fd;
} HotReloader;

HotReloader *hot_reload_init(const char *lib_path);
void *hot_reload_update(HotReloader *reloader);
void hot_reload_cleanup(HotReloader *reloader);
int watch_for_changes(HotReloader *reloader);

#endif
