#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

static struct dirent* (*oreaddir)(DIR * ) = NULL;
static ssize_t (*oreadlink)(const char *restrict, char *restrict, size_t)= NULL;
static ssize_t (*owrite)(int, const void*, size_t) = NULL; 
static int (*oopen)(const char*, int, ...) = NULL;
static int (*okill)(pid_t, int) = NULL;
static int (*ounlinkat)(int, const char*, int) = NULL;
static int (*ofputs)(const char *restrict, FILE *restrict) = NULL;
static FILE* (*ofopen)(const char *restrict, const char *restrict) = NULL;
