// Copyright (c) 2013 by Julie Kim (kjulee114@gmail.com)
#include <string.h>

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
    if (ptr->pid == pid) {
      syslog(LOG_DEBUG, "Find proc_node(%d)", pid);
      return ptr;
    }
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
    syslog(LOG_DEBUG, "print trace tree");
    print_trace_tree(ttree_ptr);
    return ttree_ptr->fd;
  }
  trace_node* tmp_ptr = ttree_ptr;
  trace_node* tmp_p_ptr = ttree_ptr;
  int f_side = 0;
  if (exist_trace_node(ttree_ptr, new_node->fd) == TRUE) {
    syslog(LOG_DEBUG, "There is already trace_node with pid:%d and fd:%d.\
        Please check your trace logs.", pid, new_node->fd);
    syslog(LOG_DEBUG, "print trace tree");
    print_trace_tree(ttree_ptr);
    return -1;
  }
  /** find position of new_node **/
  syslog(LOG_DEBUG, "Start traversal from fd:%d", tmp_ptr->fd);
  while (tmp_ptr != NULL) {
    if (tmp_ptr->fd < new_node->fd) {
      f_side = RIGHT;
      tmp_p_ptr = tmp_ptr;
      tmp_ptr = tmp_ptr->rchild;
    } else {
      f_side = LEFT;
      tmp_p_ptr = tmp_ptr;
      tmp_ptr = tmp_ptr->lchild;
    }
    ++depth;
  }
  tmp_ptr = new_node;
  tmp_ptr->rchild = NULL;
  tmp_ptr->lchild = NULL;
  if (f_side == RIGHT) {
    tmp_p_ptr->rchild = tmp_ptr;
  } else {
    tmp_p_ptr->lchild = tmp_ptr;
  }
  syslog(LOG_DEBUG, "new node fd:%d is located in %dth level",
      tmp_ptr->fd, depth);
  syslog(LOG_DEBUG, "print trace tree");
  print_trace_tree(p_ptr->trace_tree);
  return tmp_ptr->fd;
}

trace_node* find_parent_trace_node(trace_node* trace_tree, int fd) {
  trace_node* ptr = trace_tree;
  while (ptr != NULL) {
    if (ptr->fd == fd) {
      syslog(LOG_DEBUG, "Parent of (pid %ld, fd %d) is itself",
          trace_tree->trace->pid, trace_tree->fd);
      return ptr;
    } else if (ptr->fd > fd) {
      if (ptr->lchild == NULL) {
        fprintf(stderr, "Tree Fault:The %d is not in the tree of pid:%ld(l)\n",
            fd, trace_tree->trace->pid);
        return NULL;
      }
      if (ptr->lchild->fd == fd) {
        syslog(LOG_DEBUG, "Find the %d, parent fd is %d", fd, ptr->fd);
        return ptr;
      } else {
        ptr = ptr->lchild;
        syslog(LOG_DEBUG,
            "Lchild of the parent is not matched. Move to lchild");
      }
    } else if (ptr->fd < fd) {
      if (ptr->rchild == NULL) {
        fprintf(stderr, "Tree Fault:The %d is not in the tree of pid:%ld(r)\n",
            fd, trace_tree->trace->pid);
        return NULL;
      }
      if (ptr->rchild->fd == fd) {
        syslog(LOG_DEBUG, "Find the %d, parent fd is %d", fd, ptr->fd);
        return ptr;
      } else {
        ptr = ptr->rchild;
        syslog(LOG_DEBUG,
            "Rchild of the parent is not matched. Move to rchild");
      }
    }
  }
  syslog(LOG_DEBUG, "Cannot find %d from trace_tree", fd);
  return NULL;
}

trace_node* find_trace_node(trace_node* trace_tree, int fd) {
  trace_node* t_ptr = trace_tree;
  syslog(LOG_DEBUG, "enter find_trace_node with fd:%d", fd);
  while (t_ptr != NULL) {
    if (t_ptr->fd == fd) {
      syslog(LOG_DEBUG, "Find trace_node fd:%d", t_ptr->fd);
      return t_ptr;
    } else if (t_ptr->fd > fd) {
      t_ptr = t_ptr->lchild;
    } else {
      t_ptr = t_ptr->rchild;
    }
  }
  syslog(LOG_DEBUG, "Cannot find the trace_node(%d) from the tree", fd);
  return NULL;
}

int remove_trace_node(long int pid, long int fd) {
  syslog(LOG_DEBUG, "enter remove_trace_node with pid %ld, fd %ld", pid, fd);
  proc_node* cur_proc = find_proc_node(pid);
  trace_node* p_trace;

  if (cur_proc == NULL) {
    syslog(LOG_WARNING, "This proc(%ld) is not added to proc_list yet", pid);
    return pid;
  }
  syslog(LOG_DEBUG, "print trace_tree before delete");
  print_trace_tree(cur_proc->trace_tree);
  p_trace = find_parent_trace_node(cur_proc->trace_tree, fd);

  if (p_trace == NULL) {
    syslog(LOG_WARNING, "This trace(pid:%d, fd:%ld) is from another op",
        cur_proc->pid, fd);
    return cur_proc->pid;
  }
  /** case of the root **/
  if (p_trace->fd == fd) {
    syslog(LOG_DEBUG, "remove the root");
    if (p_trace->rchild != NULL) {
      trace_node* tmp = p_trace->rchild;
      trace_node* p_tmp = p_trace;
      while (tmp->lchild != NULL) {
        p_tmp = tmp;
        tmp = tmp->lchild;
      }
      if (tmp->rchild != NULL) {
        p_tmp->rchild = tmp->rchild;
      }
      tmp->rchild = p_trace->rchild->fd == tmp->fd ? NULL : p_trace->rchild;
      tmp->lchild = p_trace->lchild;
      cur_proc->trace_tree = tmp;
      syslog(LOG_DEBUG,
          "Set new root for trace_tree from right-child, pid:%ld, fd:%d",
          cur_proc->trace_tree->trace->pid, cur_proc->trace_tree->fd);
    } else if (p_trace->lchild != NULL) {
      p_trace = p_trace->lchild;
      cur_proc->trace_tree = p_trace;
      syslog(LOG_DEBUG,
          "Set new root for trace_tree from left-child, pid:%ld, fd:%d",
          cur_proc->trace_tree->trace->pid, cur_proc->trace_tree->fd);
    } else {
      syslog(LOG_DEBUG, "The tree is to empty...");
      free(cur_proc->trace_tree);
      cur_proc->trace_tree = NULL;
      //TODO(Julie) remove proc_node, too
    }
    syslog(LOG_DEBUG, "print trace tree");
    print_trace_tree(cur_proc->trace_tree);
    return cur_proc->pid;
  }

  /* check the rchild and lchild of cur_trace.
   * If it is not leaf node, reorder the tree */
  int f_side = p_trace->rchild->fd == fd ? RIGHT : LEFT; // 1:rchild, 0:lchild
  trace_node* cur_trace = f_side ? p_trace->rchild : p_trace->lchild;
  /* Reorder trace_tree from non-leaf to leaf */
  trace_node* target_ptr = NULL;
  if (cur_trace->rchild != NULL) {
    /* Select the least node from a right subtree of root */
    trace_node* ptr = cur_trace->rchild;
    if (ptr->lchild == NULL && ptr->rchild == NULL) {
      target_ptr = ptr;
    } else if (ptr->lchild == NULL && ptr->rchild != NULL) {
      target_ptr = ptr->rchild;
    } else if (ptr->lchild != NULL && ptr->rchild == NULL) {
      target_ptr = ptr->lchild;
    } else {
      trace_node* p_ptr = ptr;
      ptr = ptr->rchild;
      while (ptr->lchild != NULL) {
        p_ptr = ptr;
        ptr = ptr->lchild;
      }
      target_ptr = ptr->lchild;
      p_ptr->lchild = ptr->rchild;
    }
    /*
    for (ptr = cur_trace->rchild;
        ptr->lchild != NULL && ptr->lchild->lchild != NULL;
        ptr = ptr->lchild) { }
    target_ptr = ptr->lchild;
    if (ptr->lchild->rchild != NULL) {
      ptr->lchild = ptr->lchild->rchild;
      ptr->lchild->lchild = NULL;
      ptr->lchild->rchild = NULL;
    }
    target_ptr->rchild = cur_trace->rchild;
    target_ptr->lchild = cur_trace->lchild;
    */
    syslog(LOG_DEBUG, "Target trace to change>>pid:%ld, fd:%d",
        target_ptr->trace->pid, target_ptr->fd);
  } else if (cur_trace->lchild != NULL) {
    target_ptr = cur_trace->lchild;
    syslog(LOG_DEBUG, "Target trace to change>>pid:%ld, fd:%d",
        target_ptr->trace->pid, target_ptr->fd);
  } else {
    syslog(LOG_DEBUG, "Removing trace_node is leaf...");
    free(cur_trace);
    if (f_side == RIGHT) p_trace->rchild = NULL;
    else p_trace->lchild = NULL;
    syslog(LOG_DEBUG, "print trace tree");
    print_trace_tree(cur_proc->trace_tree);
    return p_trace->trace->pid;
  }
  if (f_side == RIGHT) {
    p_trace->rchild = target_ptr;
    syslog(LOG_DEBUG,
        "Set the target into the current as rchild>>pid:%ld, fd:%d",
        p_trace->rchild->trace->pid, p_trace->rchild->fd);
  } else {
    p_trace->lchild = target_ptr;
    syslog(LOG_DEBUG,
        "Set the target into the current as lchild>>pid:%ld, fd:%d",
        p_trace->lchild->trace->pid, p_trace->lchild->fd);
  }
  syslog(LOG_DEBUG, "complete remove the trace(pid:%ld,fd:%d) and reorder",
      cur_trace->trace->pid, cur_trace->fd);
  free(cur_trace);
  syslog(LOG_DEBUG, "print trace tree");
  print_trace_tree(cur_proc->trace_tree);
  return p_trace->trace->pid;
}

void char_replace(char s1, char s2, const char* src, char* dest) {
  int i = 0;
  int len = strlen(src);
  strncpy(dest, src, len);
  for (; i < len; i++) {
    if (src[i] == s1) {
      dest[i] = s2;
    }
  }
}
