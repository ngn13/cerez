#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct backdoor {
  pid_t   *pids;
  uint64_t count;
  bool     loaded;
} backdoor_t;

void backdoor_init(backdoor_t *b);
bool backdoor_find(backdoor_t *b, char *cmd);
bool backdoor_add(backdoor_t *b, pid_t pid);
bool backdoor_contains(backdoor_t *b, pid_t pid);
void backdoor_destroy(backdoor_t *b);
