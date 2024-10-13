#define _GNU_SOURCE

#include <dirent.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void *find_addr(char *symbol) {
  // RTLD_NEXT is not defined, unless you use #define _GNU_SOURCE
  void *address = dlsym(RTLD_NEXT, symbol);
  if (address == NULL)
    debug("cant find address");
  return address;
}

// need length (l) because string contains multiple '\0's
bool replace(char *s, char x, char y, int64_t l) {
  if (NULL == s || l == 0)
    return false;

  for (l--; l >= 0; l--) {
    if (s[l] == x)
      s[l] = y;
  }

  return true;
}

uint64_t digit_count(uint64_t val) {
  if (val < 10)
    return 1;
  return 1 + digit_count(val / 10);
}

void debug(char *fmt, ...) {
  if (!DEBUG)
    return;

  va_list args;
  va_start(args, fmt);

  printf("[cerez] ");
  vprintf(fmt, args);
  printf("\n");

  va_end(args);
}

char *strremove(char *str, const char *sub) {
  size_t len = strlen(sub);
  char  *p   = str;

  if (len <= 0)
    return str;

  while ((p = strstr(p, sub)) != NULL)
    memmove(p, p + len, strlen(p + len) + 1);

  return str;
}
