#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

void cfg_init(cfg_t *c) {
  if (NULL == c)
    return;
  bzero(c, sizeof(cfg_t));
}

void cfg_destroy(cfg_t *c) {
  if (NULL == c)
    return;
  free(c->hidden);
  config_destroy(&c->libconfig);
}

bool cfg_load(cfg_t *c) {
  if (NULL == c)
    return false;

  config_setting_t *hidden = NULL, *hidden_el = NULL;
  config_init(&c->libconfig);

  if (!config_read_file(&c->libconfig, CONFIG_FILE)) {
    debug("failed to read the config file");
    goto end;
  }

  if (!config_lookup_string(&c->libconfig, "backdoor", (void *)&c->backdoor))
    debug("failed to read the backdoor config value");

  if (!config_lookup_string(&c->libconfig, "shell", (void *)&c->shell))
    debug("failed to read the shell config value");

  if (NULL == (hidden = config_lookup(&c->libconfig, "hidden"))) {
    debug("failed to read the hidden config value");
    goto end;
  }

  int         len  = config_setting_length(hidden);
  const char *path = NULL;

  for (len--; len >= 0; len--) {
    if (NULL == (hidden_el = config_setting_get_elem(hidden, len))) {
      debug("failed to get %d. hidden path config", len);
      continue;
    }

    if (!config_setting_lookup_string(hidden_el, "path", &path)) {
      debug("failed to get %d. hidden path value", len);
      continue;
    }

    if (c->hc == 0)
      c->hidden = malloc(sizeof(char *) * (++c->hc));
    else
      c->hidden = realloc(c->hidden, sizeof(char *) * (++c->hc));

    c->hidden[c->hc - 1] = path;
  }

end:
  c->ok = true;
  return true;
}
