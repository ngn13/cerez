#pragma once

#include <libconfig.h>
#include <stdbool.h>
#include <stdint.h>

#define CONFIG_FILE "/etc/cerez.cfg"

typedef struct cfg {
  config_t     libconfig;
  const char **hidden;
  uint64_t     hc; // hidden count
  char        *backdoor;
  char        *shell;
  bool         ok;
} cfg_t;

void cfg_init(cfg_t *c);
bool cfg_load(cfg_t *c);
void cfg_destroy(cfg_t *c);
