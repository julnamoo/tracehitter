// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)

#include "defs.h"

void print_proc_list() {
  proc_node* ptr = proc_list;
  syslog(LOG_DEBUG, "current proc_list");
  for(; ptr != NULL; ptr = ptr->next_proc_node) {
    syslog(LOG_DEBUG, "pid:%d", ptr->pid);
  }
}

int exist_proc_node(int pid) {
  proc_node* ptr;
  for (ptr = proc_list; ptr != NULL; ptr = ptr->next_proc_node) {
    syslog(LOG_DEBUG, "find process:%d", ptr->pid);
    if (ptr->pid == pid) {
      print_proc_list();
      return TRUE;
    }
  }
  syslog(LOG_DEBUG, "pid:%d does not exist", pid);
  print_proc_list();
  return FALSE;
}

int add_proc_node(int pid, proc_node *new_node) {
  // First insert - Create list
  if (proc_list == NULL) {
    proc_list = proc_ptr = new_node;
    syslog(LOG_DEBUG, "The first proc_node insertion. pid:%d", proc_list->pid);
    return proc_ptr->pid;
  }
  proc_ptr->next_proc_node = new_node;
  proc_ptr = new_node;
  syslog(LOG_DEBUG, "add new process node to proc_node list. pid:%d",
      proc_ptr->pid);
  return proc_ptr->pid;
}

proc_node* find_proc_node(int pid) {
  proc_node* ptr = proc_list;

  for (; ptr != NULL; ptr = ptr->next_proc_node) {
    if (ptr->pid == pid)
      return ptr;
  }
  return NULL;
}

int exist_trace_node(trace_node *s_ptr, int fd) {
  while (s_ptr != NULL) {
    if (s_ptr->fd == fd) {
      return TRUE;
    } else if (s_ptr->fd < fd) {
      s_ptr = s_ptr->rchild;
    } else {
      s_ptr = s_ptr->lchild;
    }
  }
  syslog(LOG_DEBUG, "Cannot find fd:%d", fd);
  return FALSE;
}

int add_trace_node(int pid, trace_node *new_node) {
  int depth = 0;
  if (exist_proc_node(pid) == FALSE) {
    syslog(LOG_ERR, "There are no process with %d. Returned", pid);
    return -1;
  }
  proc_node* p_ptr = find_proc_node(pid);
  trace_node* ttree_ptr = p_ptr->trace_tree;
  if (ttree_ptr == NULL) {
    ttree_ptr = new_node;
    ttree_ptr->rchild = NULL;
    ttree_ptr->lchild = NULL;
  }
  trace_node* tmp_ptr = ttree_ptr;
  if (exist_trace_node(ttree_ptr, new_node->fd) == TRUE) {
    syslog(LOG_ERR, "There is already trace_node with pid:%d and fd:%d.\
        Please check your trace logs.", pid, new_node->fd);
    return -1;
  }
  /** find position of new_node **/
  syslog(LOG_DEBUG, "Start right traversal from fd:%d", tmp_ptr->fd);
  while (tmp_ptr != NULL) {
    if (tmp_ptr->fd < new_node->fd) {
      tmp_ptr = tmp_ptr->rchild;
    } else {
      tmp_ptr = tmp_ptr->lchild;
    }
    ++depth;
  }
  tmp_ptr = new_node;
  tmp_ptr->rchild = NULL;
  tmp_ptr->lchild = NULL;
  syslog(LOG_DEBUG, "new node fd:%d is located in %dth level",
      tmp_ptr->fd, depth);
  return tmp_ptr->fd;
  /* while (1) {
    for (; tmp_ptr != NULL && tmp_ptr->fd < new_node->fd;
        tmp_ptr = tmp_ptr->rchild) {
      syslog(LOG_DEBUG, "Current visit node:%d, target:%d",
          tmp_ptr->fd, new_node->fd);
      if (tmp_ptr->rchild == NULL) {
        tmp_ptr->rchild = new_node;
        syslog(LOG_DEBUG, "pid:%d, fd:%d is inserted as a rchild of fd:%d",
            pid, new_node->fd, tmp_ptr->fd);
        return tmp_ptr->rchild->fd;
      }
    }
    syslog(LOG_DEBUG, "Start left traversal from fd:%d", tmp_ptr->fd);
    for (; tmp_ptr != NULL && tmp_ptr->fd > new_node->fd;
        tmp_ptr = tmp_ptr->lchild) {
      syslog(LOG_DEBUG, "Current visit node:%d, target:%d",
          tmp_ptr->fd, new_node->fd);
      if (tmp_ptr->lchild == NULL) {
        syslog(LOG_DEBUG, "pid:%d, fd:%d is inserted as a lchild of fd:%d",
            pid, new_node->fd, tmp_ptr->fd);
        return tmp_ptr->lchild->fd;
      }
    }
  }*/
}
