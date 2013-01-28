// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)

#include "defs.h"

void print_proc_list() {
  proc_node* ptr = proc_list;
  syslog(LOG_DEBUG, "current proc_list");
  for(; ptr != NULL; ptr = ptr->next_proc_node) {
    syslog(LOG_DEBUG, "pid:%d", ptr->pid);
  }
}

void print_trace_tree(trace_node* trace_tree) {
  if (trace_tree == NULL) {
    syslog(LOG_DEBUG, "End of trace_tree");
    return;
  }
  trace *t_ptr = trace_tree->trace;
  syslog(LOG_DEBUG, "print trace_node in recursively.");
  syslog(LOG_DEBUG, "pid %ld, fd %ld, fname %s, state %d, rval %ld",
      t_ptr->pid, t_ptr->fd, t_ptr->fname, t_ptr->state, t_ptr->rval);
  print_trace_tree(trace_tree->rchild);
  print_trace_tree(trace_tree->lchild);
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
  syslog(LOG_DEBUG, "enter find_proc_node with %d", pid);
  proc_node* ptr = proc_list;

  for (; ptr != NULL; ptr = ptr->next_proc_node) {
    if (ptr->pid == pid)
      return ptr;
  }
  syslog(LOG_DEBUG, "Cannot find proc_node(%d) from list (@find_proc_node)",
      pid);
  return NULL;
}

int exist_trace_node(trace_node *s_ptr, int fd) {
  while (s_ptr != NULL) {
    if (s_ptr->fd == fd) {
      syslog(LOG_DEBUG, "Success to find trace_node pid %ld, fd %d",
          s_ptr->trace->pid, fd);
      return TRUE;
    } else if (s_ptr->fd < fd) {
      syslog(LOG_DEBUG, "go to rchild from pid %ld, fd %d",
          s_ptr->trace->pid, s_ptr->fd);
      s_ptr = s_ptr->rchild;
    } else {
      syslog(LOG_DEBUG, "go to lchild from pid %ld, fd %d",
          s_ptr->trace->pid, s_ptr->fd);
      s_ptr = s_ptr->lchild;
    }
  }
  syslog(LOG_DEBUG, "Cannot find fd:%d", fd);
  return FALSE;
}

int add_trace_node(int pid, trace_node *new_node) {
  syslog(LOG_DEBUG, "enter add_trace_node");
  int depth = 0;
  if (exist_proc_node(pid) == FALSE) {
    syslog(LOG_DEBUG, "There are no process with %d. Returned", pid);
    return -1;
  }
  proc_node* p_ptr = find_proc_node(pid);
  trace_node* ttree_ptr = p_ptr->trace_tree;
  if (ttree_ptr == NULL) {
    ttree_ptr = new_node;
    syslog(LOG_DEBUG, "Is it really the first trace_node of %d(pid)?", pid);
    ttree_ptr->rchild = NULL;
    ttree_ptr->lchild = NULL;
    print_trace_tree(ttree_ptr);
    return ttree_ptr->fd;
  }
  trace_node* tmp_ptr = ttree_ptr;
  if (exist_trace_node(ttree_ptr, new_node->fd) == TRUE) {
    syslog(LOG_DEBUG, "There is already trace_node with pid:%d and fd:%d.\
        Please check your trace logs.", pid, new_node->fd);
    print_trace_tree(ttree_ptr);
    return -1;
  }
  /** find position of new_node **/
  syslog(LOG_DEBUG, "Start traversal from fd:%d", tmp_ptr->fd);
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
  print_trace_tree(p_ptr->trace_tree);
  return tmp_ptr->fd;
}

trace_node* find_parent_trace_node(trace_node* trace_tree, int fd) {
  //TODO(Julie)
}

int remove_trace_node(long int pid, long int fd) {
  syslog(LOG_DEBUG, "enter remove_trace_node with pid %ld, fd %ld", pid, fd);
  proc_node* cur_proc = find_proc_node(pid);
  trace_node* p_trace = find_parent_trace_node(cur_proc->trace_tree, fd);
  /* check the rchild and lchild of cur_trace.
   * If it is not leaf node, reorder the tree */
  int f_side = p_trace->rchild->fd == fd ? 1 : 0; // 1:rchild, 0:lchild
  trace_node* cur_trace =  f_side ? p_trace->rchild : p_trace->lchild;
  //TODO(Julie) Reorder trace_tree
}
