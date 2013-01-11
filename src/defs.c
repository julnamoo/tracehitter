// Copyright (c) 2013 Julie Kim (julie114.kim@gmail.com)
// Julie Kim (kjulee114@gmail.com)

#include "defs.h"

int exist_proc_node(int pid) {
  proc_node* ptr;
  for (ptr = proc_list; ptr != NULL; ptr = ptr->next_proc_node) {
    if (ptr->pid == pid)
      return TRUE;
  }
  return FALSE;
}
