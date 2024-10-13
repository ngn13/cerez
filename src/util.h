#pragma once

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void     debug(char *fmt, ...);
void    *find_addr(char *symbol);
void     get_originals();
bool     replace(char *s, char x, char y, int64_t l);
uint64_t digit_count(uint64_t val);
char    *strremove(char *str, const char *sub);
