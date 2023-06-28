#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#define DEBUG 1

int debug(char* str){
  if(DEBUG){
    printf("%s\n", str);
    return 0;
  }
  return 1;
}
