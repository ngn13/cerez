#ifndef CONFIG_H
#define CONFIG_H

#include <libconfig.h>
#include <stdlib.h>
#include <stdio.h>

struct Config {
  const char** hidden;
  const char* backdoor;
  int hidden_count; 
};

int init_cfg(struct Config*);
void clean_cfg(struct Config*);
int read_cfg(struct Config*);

#endif
