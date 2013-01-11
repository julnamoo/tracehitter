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

int add_proc_node(int pid, proc_node *new_node) {
  proc_node* ptr  = proc_list;
  while (ptr != NULL) {
    ptr = ptr->next_proc_node;
  }
  ptr = new_node;
  syslog(LOG_DEBUG, "add new process node to proc_node list. pid:%d", ptr->pid);
  return ptr->pid;
}
