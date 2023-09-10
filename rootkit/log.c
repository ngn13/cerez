#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

int debug(char* str){
  if(DEBUG){
    printf("[CEREZ-DEBUG] %s\n", str);
    return 0;
  }
  return 1;
}
