#include "hotr.h"
#include <dlfcn.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

HotReloader *hot_reload_init(const char *lib_path) {
  HotReloader *reloader = malloc(sizeof(HotReloader));

  reloader->lib_path = strdup(lib_path);
  reloader->handle = NULL;
  reloader->last_modified = 0;
  reloader->cleanup = NULL;

  reloader->inotify_fd = inotify_init();
  if (reloader->inotify_fd < 0) {
    perror("inotify_init error");
    free(reloader->lib_path);
    free(reloader);
    return NULL;
  }

  char *dir_path = dirname(strdup(lib_path));
  reloader->watch_fd = inotify_add_watch(reloader->inotify_fd, dir_path,
                                         IN_MODIFY | IN_CREATE | IN_MOVED_TO);
  free(dir_path);

  if (reloader->watch_fd < 0) {
    perror("inotify_add_watch");
    close(reloader->inotify_fd);
    free(reloader->lib_path);
    free(reloader);
    return NULL;
  }

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
  usleep(100000);

  struct stat attr;
  if (stat(reloader->lib_path, &attr) != 0 || attr.st_size == 0) {
    fprintf(stderr, "Library file not ready\n");
    return NULL;
  }
  if (reloader->cleanup != NULL) {
    reloader->cleanup();
  }

  if (reloader->handle != NULL) {
    dlclose(reloader->handle);
    reloader->handle = NULL;
  }

  reloader->handle = dlopen(reloader->lib_path, RTLD_NOW);
  if (!reloader->handle) {
    fprintf(stderr, "Error loading library: %s\n", dlerror());
    return NULL;
  }

  reloader->last_modified = attr.st_mtime;

  reloader->cleanup = dlsym(reloader->handle, "cleanup");

  return reloader->handle;
}

int watch_for_changes(HotReloader *reloader) {
  char buffer[BUF_LEN];
  char *lib_name = basename(strdup(reloader->lib_path));

  int length = read(reloader->inotify_fd, buffer, BUF_LEN);
  if (length < 0) {
    perror("read");
    free(lib_name);
    return -1;
  }

  int i = 0;
  while (i < length) {
    struct inotify_event *event = (struct inotify_event *)&buffer[i];
    if (event->len && !(event->mask & IN_ISDIR)) {
      if (strcmp(event->name, lib_name) == 0) {
        free(lib_name);
        return 1; // File changed
      }
    }
    i += EVENT_SIZE + event->len;
  }

  free(lib_name);
  return 0; // No relevant changes
}

void hot_reload_cleanup(HotReloader *reloader) {
  if (reloader->cleanup != NULL) {
    reloader->cleanup();
  }
  if (reloader->handle != NULL) {
    dlclose(reloader->handle);
  }
  if (reloader->watch_fd >= 0) {
    inotify_rm_watch(reloader->inotify_fd, reloader->watch_fd);
  }
  if (reloader->inotify_fd >= 0) {
    close(reloader->inotify_fd);
  }
  free(reloader->lib_path);
  free(reloader);
}
